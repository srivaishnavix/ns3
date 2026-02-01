#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-apps-module.h"

using namespace ns3;

int main ()
{
  // 1. Create nodes
  NodeContainer nodes;
  nodes.Create (2);

  Ptr<Node> ground = nodes.Get (0);
  Ptr<Node> satellite = nodes.Get (1);

  // 2. LEO-like link (high delay, high bandwidth)
  PointToPointHelper leoLink;
  leoLink.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  leoLink.SetChannelAttribute ("Delay", StringValue ("30ms")); // ~LEO RTT/2

  NetDeviceContainer devices = leoLink.Install (nodes);

  // 3. Fixed positions (for now)
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  // 4. Internet stack
  InternetStackHelper internet;
  internet.Install (nodes);

  // 5. IP addressing
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.10.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  // 6. Ping from ground → satellite
  PingHelper ping (interfaces.GetAddress (1));
  ping.Install (ground);

  // 7. Run simulation
  Simulator::Stop (Seconds (6.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
