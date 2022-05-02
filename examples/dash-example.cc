/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Jaume Nin <jaume.nin@cttc.cat>
 *          Manuel Requena <manuel.requena@cttc.es>
 */

#include <ns3/dash-module.h>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/lte-module.h"
#include "ns3/lte-ue-phy.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeBs,
 * attaches one UE per eNodeB starts a flow for each UE to and from a remote host.
 * It also starts another flow between each UE pair.
 */

NS_LOG_COMPONENT_DEFINE ("LenaSimpleEpc");

int
main (int argc, char *argv[])
{
  uint16_t numUeNodes = 200;
  uint16_t numEnbNodes = 20;
  uint16_t rapUeEnb = numUeNodes / numEnbNodes;
  uint16_t numNodePairs = 2;
  double stopTime = 3600.0;
  Time simTime = MilliSeconds (1100);
  double distance = 40.0;
  Time interPacketInterval = MilliSeconds (100);
  bool useCa = false;
  bool disableDl = true;
  bool disableUl = true;
  bool disablePl = true;
  bool useDash = true;
  double targetDt = 35.0;
  double window = 10.0;
  uint32_t bufferSpace = 30000000;

  // Command line arguments
  CommandLine cmd (__FILE__);
  cmd.AddValue ("numNodePairs", "Number of eNodeBs + UE pairs", numNodePairs);
  cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue ("distance", "Distance between eNBs [m]", distance);
  cmd.AddValue ("interPacketInterval", "Inter packet interval", interPacketInterval);
  cmd.AddValue ("useCa", "Whether to use carrier aggregation.", useCa);
  cmd.AddValue ("disableDl", "Disable downlink data flows", disableDl);
  cmd.AddValue ("disableUl", "Disable uplink data flows", disableUl);
  cmd.AddValue ("disablePl", "Disable data flows between peer UEs", disablePl);
  cmd.AddValue ("useDash", "Use dash protocol", useDash);
  cmd.AddValue ("targetDt", "The target time difference between receiving and playing a frame.", targetDt);
  cmd.AddValue ("window", "The window for measuring the average throughput.", window);
  cmd.AddValue ("bufferSpace", "The space in bytes that is used for buffering the video", bufferSpace);
  cmd.Parse (argc, argv);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);

  if (useCa)
   {
     Config::SetDefault ("ns3::LteHelper::UseCa", BooleanValue (useCa));
     Config::SetDefault ("ns3::LteHelper::NumberOfComponentCarriers", UintegerValue (2));
     Config::SetDefault ("ns3::LteHelper::EnbComponentCarrierManager", StringValue ("ns3::RrComponentCarrierManager"));
   }
  std::cout << "index,dim_packets,count_packet,time,node_id,new_bit_rate,old_bit_rate,est_bit_rate,inter_time,throughput" << std::endl;
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create (numEnbNodes);
  ueNodes.Create (numUeNodes);

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numEnbNodes/2; i++)
    {
      positionAlloc->Add (Vector (1000 * i, 2500, 0));
      positionAlloc->Add (Vector (1000 * i, 7500, 0));
    }
  
  MobilityHelper mobilityenb;
  mobilityenb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityenb.SetPositionAllocator(positionAlloc);
  mobilityenb.Install(enbNodes);

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Mode", StringValue ("Time"),
                             "Time", StringValue ("2s"),
                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=5.0]"),
                             "Bounds", StringValue ("0|10000|0|10000"));
  //mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator", "X", StringValue ("100.0"), "Y", StringValue ("100.0"), "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=30]"));
  //mobility.Install(enbNodes);
  Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numUeNodes; i++)
    {
      positionAlloc2->Add (Vector (distance * i, distance * i, 0));
    }
  mobility.SetPositionAllocator(positionAlloc2);
  mobility.Install(ueNodes);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  if (numUeNodes > numEnbNodes){
    // Attach one UE per eNodeB
    for (uint16_t i = 0; i < numEnbNodes; i++)
    {
      for (uint16_t j = 0; j < numUeNodes/numEnbNodes; j++) {
        lteHelper->Attach (ueLteDevs.Get(j + (i * rapUeEnb)), enbLteDevs.Get(i));
      }
    }
  } else {
    for (uint16_t i = 0; i < numUeNodes; i++)
    {
      lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
    }
  }
  

  
  // Install and start applications on UEs and remote host
  uint16_t dlPort = 1100;
  uint16_t ulPort = 2000;
  uint16_t otherPort = 3000;
  uint16_t dashPort = 15000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ue = ueNodes.Get (u);
      if (!disableDl)
        {
          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));

          UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
          dlClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
          dlClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
          clientApps.Add (dlClient.Install (remoteHost));
        }

      if (!disableUl)
        {
          ++ulPort;
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

          UdpClientHelper ulClient (remoteHostAddr, ulPort);
          ulClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
          ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
          clientApps.Add (ulClient.Install (ueNodes.Get(u)));
        }

      if (!disablePl && numNodePairs > 1)
        {
          ++otherPort;
          PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
          serverApps.Add (packetSinkHelper.Install (ueNodes.Get (u)));

          UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
          client.SetAttribute ("Interval", TimeValue (interPacketInterval));
          client.SetAttribute ("MaxPackets", UintegerValue (1000000));
          clientApps.Add (client.Install (ueNodes.Get ((u + 1) % numNodePairs)));
        }
      if (useDash)
        {
          DashClientHelper dashClientHelper ("ns3::TcpSocketFactory",
                                              InetSocketAddress (remoteHostAddr, dashPort),
                                              "ns3::FdashClient");

          dashClientHelper.SetAttribute (
                      "VideoId", UintegerValue (u + 1)); // VideoId should be positive
          dashClientHelper.SetAttribute ("TargetDt", TimeValue (Seconds (targetDt)));
          dashClientHelper.SetAttribute ("window", TimeValue (Seconds (window)));
          dashClientHelper.SetAttribute ("bufferSpace", UintegerValue (bufferSpace));

          clientApps.Add (dashClientHelper.Install (ue));
          clientApps.Start (Seconds(2.0));
          clientApps.Stop (Seconds(stopTime));
        }
    }
  if (useDash)
  {
    ApplicationContainer remoteApps;
    DashServerHelper dashServerHelper ("ns3::TcpSocketFactory",
                                        InetSocketAddress (Ipv4Address::GetAny (), dashPort));
    remoteApps.Add (dashServerHelper.Install (remoteHost));
    remoteApps.Start (Seconds (0.0));
    remoteApps.Stop (Seconds (stopTime + 5.0));
  }
  else 
  {
    serverApps.Start (MilliSeconds (500));
    clientApps.Start (MilliSeconds (500));
  }

  lteHelper->EnableTraces ();
  AsciiTraceHelper ascii;
  mobility.EnableAsciiAll(ascii.CreateFileStream("Mobilityue-Trace-example.mob"));
  mobilityenb.EnableAsciiAll(ascii.CreateFileStream("Mobilityenb-Trace-example.mob"));

  // Uncomment to enable PCAP tracing
  p2ph.EnablePcapAll("lte-simple");
  Simulator::Stop (Seconds (stopTime));
  Simulator::Run ();
  Simulator::Destroy ();

  /*
  uint32_t users = clientApps.GetN ();
  uint32_t k;
  for (k = 0; k < users; k++)
    {
      Ptr<DashClient> app = DynamicCast<DashClient> (clientApps.Get (k));
      std::cout << "ns3::FdashClient" << "-Node: " << k;
      app->GetStats ();
    }
  */
  //GtkConfigStore config;
  //config.ConfigureAttributes ();

  lteHelper = 0;  // forse va tolto
  return 0;
}
