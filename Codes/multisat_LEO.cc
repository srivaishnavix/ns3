#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/propagation-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("SatelliteSim");


int main ()
{
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

 
    std::cout << "Simulation Started" << std::endl;

  // ----- Create nodes -----
  NodeContainer groundStation;
  groundStation.Create (1);

  NodeContainer satellites;
  satellites.Create (3);

  NodeContainer allNodes;
  allNodes.Add (groundStation);
  allNodes.Add (satellites);

  // ----- Mobility -----
  MobilityHelper mobility;

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  // Ground station at Earth surface
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));

  // LEO Satellites at different altitudes
  positionAlloc->Add (Vector (0.0, 0.0, 500000.0));  // 500 km
  positionAlloc->Add (Vector (200000.0, 0.0, 700000.0)); // 700 km
  positionAlloc->Add (Vector (-200000.0, 0.0, 900000.0)); // 900 km

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (allNodes);

  // ----- Multi-Ray Approximation Channel -----
  Ptr<YansWifiChannel> channel = CreateObject<YansWifiChannel> ();

  // Friis Free Space Loss
  Ptr<FriisPropagationLossModel> friis = CreateObject<FriisPropagationLossModel> ();

  // Log Distance Shadowing
  Ptr<LogDistancePropagationLossModel> logDistance = CreateObject<LogDistancePropagationLossModel> ();
  logDistance->SetReference (1.0, 40.0);

  // Random Obstruction Loss (Airplane / Debris simulation)
  Ptr<RandomPropagationLossModel> obstruction = CreateObject<RandomPropagationLossModel> ();
  obstruction->SetAttribute ("Variable", StringValue ("ns3::UniformRandomVariable[Min=0|Max=10]"));

  // Chain propagation models
  friis->SetNext (logDistance);
  logDistance->SetNext (obstruction);

  channel->SetPropagationLossModel (friis);
  channel->SetPropagationDelayModel (CreateObject<ConstantSpeedPropagationDelayModel> ());

  // ----- WiFi PHY -----
  YansWifiPhyHelper phy;
  phy.SetChannel (channel);

  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211ac);

  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");

  NetDeviceContainer devices = wifi.Install (phy, mac, allNodes);

  // ----- Internet Stack -----
  InternetStackHelper internet;
  internet.Install (allNodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  // ----- Traffic (Ground → Satellite 1) -----
  UdpEchoServerHelper server (9);
  ApplicationContainer serverApp = server.Install (satellites.Get (0));
  serverApp.Start (Seconds (1.0));
  serverApp.Stop (Seconds (10.0));

  UdpEchoClientHelper client (interfaces.GetAddress (1), 9);
  client.SetAttribute ("MaxPackets", UintegerValue (20));
  client.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  client.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApp = client.Install (groundStation.Get (0));
  clientApp.Start (Seconds (2.0));
  clientApp.Stop (Seconds (10.0));

  // ----- Run -----
  Simulator::Stop (Seconds (30.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
