#include "ns3/internet-apps-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  Time::SetResolution (Time::NS);

  // 1. Create 2 nodes
  NodeContainer nodes;
  nodes.Create (2);

  // 2. Create a point-to-point link
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("9ms"));

  NetDeviceContainer devices = p2p.Install (nodes);

  // 3. Install Internet stack (TCP/IP)
  InternetStackHelper internet;
  internet.Install (nodes);

  // 4. Assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  // 5. Install ping application on node 0
  PingHelper ping (interfaces.GetAddress (1));
//  ping.SetAttribute ("Verbose", BooleanValue (true));

  ApplicationContainer apps = ping.Install (nodes.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (5.0));

  // 6. Run simulation
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
