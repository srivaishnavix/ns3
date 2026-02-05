#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/propagation-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EphemerisSatelliteSim");

// ----- Convert Lat/Lon/Alt to ECEF -----
static Vector
LatLonAltToECEF(double latDeg, double lonDeg, double altMeters)
{
  double R = 6371000.0;

  double lat = latDeg * M_PI / 180.0;
  double lon = lonDeg * M_PI / 180.0;

  double x = (R + altMeters) * cos(lat) * cos(lon);
  double y = (R + altMeters) * cos(lat) * sin(lon);
  double z = (R + altMeters) * sin(lat);

  return Vector(x, y, z);
}

int main ()
{
  std::cout << "Simulation Started" << std::endl;

  // ----- Enable Logging -----
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

  // ----- Create Nodes -----
  NodeContainer ground;
  ground.Create (1);

  NodeContainer satellites;
  satellites.Create (3);

  NodeContainer allNodes;
  allNodes.Add (ground);
  allNodes.Add (satellites);

  // ----- Mobility -----
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator>();

  // Ground Station
  Vector gs = LatLonAltToECEF(17.3850, 78.4867, 0);
  posAlloc->Add(gs);

  // Satellites
  posAlloc->Add(LatLonAltToECEF(17.3850, 78.4867, 500000));
  posAlloc->Add(LatLonAltToECEF(17.3850, 78.5867, 600000));
  posAlloc->Add(LatLonAltToECEF(17.4850, 78.4867, 800000));

  mobility.SetPositionAllocator(posAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(allNodes);

  // ----- Print Node Positions -----
  for (uint32_t i = 0; i < allNodes.GetN(); i++)
  {
    Ptr<MobilityModel> mob = allNodes.Get(i)->GetObject<MobilityModel>();
    Vector pos = mob->GetPosition();

    std::cout << "Node " << i
              << " Position: "
              << pos.x << ", "
              << pos.y << ", "
              << pos.z << std::endl;
  }

  // ----- Channel -----
  Ptr<YansWifiChannel> channel = CreateObject<YansWifiChannel>();

  Ptr<FriisPropagationLossModel> friis = CreateObject<FriisPropagationLossModel>();
  channel->SetPropagationLossModel(friis);
  channel->SetPropagationDelayModel(CreateObject<ConstantSpeedPropagationDelayModel>());

  // ----- PHY -----
  YansWifiPhyHelper phy;
  phy.SetChannel(channel);

  // Increase transmit power for satellite distance
  phy.Set("TxPowerStart", DoubleValue(40.0));
  phy.Set("TxPowerEnd", DoubleValue(40.0));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211ac);

  WifiMacHelper mac;
  mac.SetType("ns3::AdhocWifiMac");

  NetDeviceContainer devices = wifi.Install(phy, mac, allNodes);

  // ----- Enable PCAP Tracing -----
  phy.EnablePcap("ephemeris-sat", devices);

  // ----- Internet -----
  InternetStackHelper internet;
  internet.Install(allNodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.2.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  // ----- Applications -----
  UdpEchoServerHelper server(9);
  ApplicationContainer serverApp = server.Install(satellites.Get(0));
  serverApp.Start(Seconds(1));
  serverApp.Stop(Seconds(20));

  UdpEchoClientHelper client(interfaces.GetAddress(1), 9);
  client.SetAttribute("MaxPackets", UintegerValue(30));
  client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  client.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApp = client.Install(ground.Get(0));
  clientApp.Start(Seconds(2));
  clientApp.Stop(Seconds(20));

  // ----- Simulation -----
  Simulator::Stop(Seconds(25));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
