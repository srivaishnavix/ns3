#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-apps-module.h"

using namespace ns3;

int main ()
{
  // 1. Create two nodes
  NodeContainer nodes;
  nodes.Create (2);

  // 2. Wi-Fi channel and PHY
  YansWifiChannelHelper channel;
  channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss ("ns3::FriisPropagationLossModel");

  YansWifiPhyHelper phy;

  phy.SetChannel (channel.Create ());

  // 3. Wi-Fi setup
  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211b);

  WifiMacHelper mac;
  Ssid ssid = Ssid ("simple-wifi");

  // Node 0 = AP
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevice = wifi.Install (phy, mac, nodes.Get (0));

  // Node 1 = STA
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevice = wifi.Install (phy, mac, nodes.Get (1));

  // 4. Mobility (fixed positions)
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  // 5. Internet stack
  InternetStackHelper internet;
  internet.Install (nodes);

  // 6. IP addressing
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");

  ipv4.Assign (apDevice);
  Ipv4InterfaceContainer staInterface = ipv4.Assign (staDevice);

  // 7. Ping from STA → AP
  PingHelper ping (Ipv4Address ("10.1.1.1"));
  ping.Install (nodes.Get (1));

  // 8. Run simulation
  Simulator::Stop (Seconds (4.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
