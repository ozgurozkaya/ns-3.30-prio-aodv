/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <iostream>
#include <string>
#include <cmath>
#include "ns3/aodv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/ssid.h"
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/onoff-application.h"
#include "ns3/random-variable-stream.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/rng-seed-manager.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("VanetRC");

class RoutingExample{
  public:
    void run();
    // argc & argv configuration
    void configuration(int argc, char ** argv);

  private:
    // parameters
    uint32_t server_node, client_node;
    // Simulation time, seconds
    double totalTime = 180;
    double duration;
    // Write per-device PCAP traces if true
    bool pcap = true;
    // Print routes if true
    bool printRoutes = true;
    //Size of Packet (bytes), Packet interval (Time), Max Packets
    double packet_size = 1024;
    Time packet_interval = MilliSeconds (1000);
    double max_packets = 250;
    StringValue data_rate = StringValue("150kb/s");
    //Internet Stack Helper
    InternetStackHelper stack;
    //Pointer to the packet sink application 
    Ptr<PacketSink> sink[100];
    //The value of the last total received bytes
    uint64_t lastTotalRx = 0;
    //Routing Method
    AodvHelper routing;

  // network
  // nodes used in the example
  NodeContainer nodes;
  // devices used in the example
  NetDeviceContainer devices;
  // interfaces used in the example
  Ipv4InterfaceContainer interfaces;

  private:
    // Create the nodes ()
    void createNodes ();
    // Create the devices
    void createDevices ();
    // Create the network
    void installInternetStack ();
    // Create the simulation applications
    void installApplications ();
    void installOnOffApplications ();
    // Saves all nodes' routing tables in a txt file
    void printingRoutingTable ();
    // Saves all nodes' pcap tracing file
    void enablePcapTracing ();
    // Calculate the throughput of network
    void calculateThroughput();
    // Change node emergency status
    void makeEmergencyNode(int i);
    // Checks array
    bool checkArray(uint32_t array[], uint32_t value);
    bool checkArray(Ipv4Address array[], Ipv4Address value);
    
};

void
RoutingExample::run(){
  
  std::string animFile = "xml/demo/animation.xml";
  // Enable logging for UdpClient and
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  createNodes();
  createDevices();
  installInternetStack();
  installOnOffApplications();
  if(pcap) enablePcapTracing();
  if(printRoutes) printingRoutingTable();
  
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();
 
  // animation for NetAnim

  AnimationInterface animation (animFile);
  animation.SetMaxPktsPerTraceFile(5000000);
  
  animation.UpdateNodeDescription(nodes.Get(0), "Source 1");
  
  animation.UpdateNodeDescription(nodes.Get(1), "Source 2 (Emergency)");
  animation.UpdateNodeColor(nodes.Get(1), uint8_t(0), uint8_t(0), uint8_t(255));
  animation.UpdateNodeDescription(nodes.Get(2), "Source 3");

  animation.UpdateNodeDescription(nodes.Get(3), "Intermediate Node (Center)");

  animation.UpdateNodeDescription(nodes.Get(4), "Intermediate Node (Emergency)");
  animation.UpdateNodeColor(nodes.Get(4), uint8_t(0), uint8_t(0), uint8_t(255));
  animation.UpdateNodeDescription(nodes.Get(5), "Intermediate Node (Emergency)");
  animation.UpdateNodeColor(nodes.Get(5), uint8_t(0), uint8_t(0), uint8_t(255));
  
  animation.UpdateNodeDescription(nodes.Get(6), "Intermediate Node");
  animation.UpdateNodeDescription(nodes.Get(7), "Intermediate Node");
  animation.UpdateNodeDescription(nodes.Get(8), "Intermediate Node");
  animation.UpdateNodeDescription(nodes.Get(9), "Intermediate Node");

  animation.UpdateNodeDescription(nodes.Get(10), "Destination");

  animation.EnablePacketMetadata (); // Optional
  animation.EnableIpv4RouteTracking ("xml/demo/vanetRC-routingtable.xml",Seconds(10), Seconds(50), Seconds(1));
  /*
  animation.EnableWifiMacCounters (Seconds (0), Seconds (totalTime)); //Optional
  animation.EnableWifiPhyCounters (Seconds (0), Seconds (totalTime)); //Optional
  animation.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (totalTime)); // Optional
  */
  
  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();

  std::cout << "\n------------------------------Measurement With 'SinkHelper'-------------------------------------\n\n";

  std::cout << " Packet Size: \t\t" << packet_size << " Bytes, " << packet_size / 1024 << " KiloBytes \n";
  std::cout << " Data Rate: \t\t" << data_rate.Get() << "\n\n";

  std::cout << "\n--------------------------------------------------------------------------------------------\n";

  // Define variables to calculate the metrics
  int k=0;
  int totaltxPackets = 0;
  int totalrxPackets = 0;
  int totaltxPackets_emergency = 0;
  int totalrxPackets_emergency = 0;
  int totaltxPacketsR = 0;
  int totalrxPacketsR = 0;
  double totaltxbytes = 0;
  double totalrxbytes = 0;
  double totaltxbytesR = 0;
  double totalrxbytesR = 0;
  double totaldelay = 0;
  double totaldelay_emergency = 0;
  double totalrxbitrate = 0;
  double totalrxbitrate_emergency = 0;
  double difftx, diffrx;
  double pdf_value, pdf_value_emergency, rxbitrate_value, rxbitrate_value_emergency, txbitrate_value, txbitrate_value_emergency, delay_value, delay_value_emergency;
  double pdf_total, pdf_total_emergency, rxbitrate_total, rxbitrate_total_emergency, delay_total, delay_total_emergency;
  double RL_rx_pack, RL_tx_pack, RL_rx_bytes, RL_tx_bytes;
  double inf = std::numeric_limits<double>::infinity();

  flowMonitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowHelper.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
  {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

      difftx = i->second.timeLastTxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
      diffrx = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstRxPacket.GetSeconds();
      pdf_value = (double) i->second.rxPackets / (double) i->second.txPackets * 100;
      txbitrate_value = (double) i->second.txBytes * 8 / 1024 / difftx;
      if (i->second.rxPackets != 0){
          rxbitrate_value = (double) i->second.rxPackets * packet_size * 8 / 1024 / diffrx;
          delay_value = (double) i->second.delaySum.GetSeconds() / (double) i->second.rxPackets;
      }
      else{
          rxbitrate_value = 0;
          delay_value = 0;
      }
      
      // We are only interested in the metrics of the data flows. This AODV
      // implementation create other flows with routing information at low bitrates,
      // so a margin is defined to ensure that only our data flows are filtered.
      if ( (!t.destinationAddress.IsSubnetDirectedBroadcast("255.255.255.0")) && (txbitrate_value > 150/1.2) && (rxbitrate_value < 150*1.2) && txbitrate_value != inf)
      {
          k++;
          
          std::cout << "\nFlow " << k << " (" << t.sourceAddress << " -> "
          << t.destinationAddress << ")\n";
          std::cout << "PDF: " << pdf_value << " %\n";
          std::cout << "Average delay: " << delay_value << "s\n";
          std::cout << "Rx bitrate: " << rxbitrate_value << " kbps\n";
          std::cout << "Tx bitrate: " << txbitrate_value << " kbps\n\n";
          
          // Acumulate for average statistics
          totaltxPackets += i->second.txPackets;
          totaltxbytes += i->second.txBytes;
          totalrxPackets += i->second.rxPackets;
          totaldelay += i->second.delaySum.GetSeconds();
          totalrxbitrate += rxbitrate_value;
          totalrxbytes += i->second.rxBytes;
      }
      else{
          totaltxbytesR += i->second.txBytes;
          totalrxbytesR += i->second.rxBytes;
          totaltxPacketsR += i->second.txPackets;
          totalrxPacketsR += i->second.rxPackets;
      }
  }

  //Average all nodes statistics
  if (totaltxPackets != 0){
      pdf_total = (double) totalrxPackets / (double) totaltxPackets * 100;
      RL_tx_pack = (double) totaltxPacketsR / (double) totaltxPackets;
      RL_tx_bytes = totaltxbytesR / totaltxbytes;
  }
  else{
      pdf_total = 0;
      RL_tx_pack = 0;
      RL_tx_bytes = 0;
  }
  if (totalrxPackets != 0){
      rxbitrate_total = totalrxbitrate;
      delay_total = (double) totaldelay / (double) totalrxPackets;
      RL_rx_pack = (double) totalrxPacketsR / (double) totalrxPackets;
      RL_rx_bytes = totalrxbytesR / totalrxbytes;
  }
  else{
      rxbitrate_total = 0;
      delay_total = 0;
      RL_rx_pack = 0;
      RL_rx_bytes = 0;
  }

  if (totaltxPackets_emergency != 0){
      pdf_total_emergency = (double) totalrxPackets_emergency / (double) totaltxPackets_emergency * 100;
  }

  if (totalrxPackets != 0){
      rxbitrate_total_emergency = totalrxbitrate_emergency;
      delay_total_emergency = (double) totaldelay_emergency / (double) totalrxPackets_emergency;
  }
  
  // Print all nodes statistics
  std::cout << "\nTotal PDF: " << pdf_total << " %\n";
  std::cout << "Total Rx bitrate: " << rxbitrate_total << " kbps\n";
  std::cout << "Total Delay: " << delay_total << " s\n";

  Simulator::Destroy ();
};

void RoutingExample::configuration(int argc, char ** argv){
  CommandLine cmd;
  cmd.Parse (argc, argv);
}

void
RoutingExample::createNodes(){
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Mode", StringValue ("Time"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Time", StringValue ("2s"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  //Creating nodes
  nodes.Create(11);

  //Adding Mobility to the created nodes
  MobilityHelper mobility;

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (nodes);
  nodes.Get(0)->GetObject<MobilityModel>()->SetPosition(Vector3D(50,0,0));
  nodes.Get(1)->GetObject<MobilityModel>()->SetPosition(Vector3D(85,0,0));
  
  nodes.Get(2)->GetObject<MobilityModel>()->SetPosition(Vector3D(120,0,0));

  nodes.Get(3)->GetObject<MobilityModel>()->SetPosition(Vector3D(85,40,0));

  nodes.Get(4)->GetObject<MobilityModel>()->SetPosition(Vector3D(50,80,0));
  nodes.Get(5)->GetObject<MobilityModel>()->SetPosition(Vector3D(50,120,0));

  nodes.Get(6)->GetObject<MobilityModel>()->SetPosition(Vector3D(120,80,0));
  nodes.Get(7)->GetObject<MobilityModel>()->SetPosition(Vector3D(150,100,0));
  nodes.Get(8)->GetObject<MobilityModel>()->SetPosition(Vector3D(170,130,0));
  nodes.Get(9)->GetObject<MobilityModel>()->SetPosition(Vector3D(130,180,0));

  nodes.Get(10)->GetObject<MobilityModel>()->SetPosition(Vector3D(85,160,0));
};

void
RoutingExample::createDevices(){
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();

  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");

  wifiPhy.Set ("RxSensitivity", DoubleValue (-89.0) );
  wifiPhy.Set ("CcaEdThreshold", DoubleValue (-62.0) );
  wifiPhy.Set ("TxGain", DoubleValue (1.0) );
  wifiPhy.Set ("RxGain", DoubleValue (1.0) );
  wifiPhy.Set ("TxPowerLevels", UintegerValue (1) );
  wifiPhy.Set ("TxPowerEnd", DoubleValue (18) );
  wifiPhy.Set ("TxPowerStart", DoubleValue (18) );
  wifiPhy.Set ("RxNoiseFigure", DoubleValue (7.0) );
  wifiPhy.SetChannel (wifiChannel.Create ());
  
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));

  devices = wifi.Install (wifiPhy, wifiMac, nodes); 
};

void
RoutingExample::installInternetStack(){
  
  //routing.Set ("DestinationOnly", BooleanValue (true));
  routing.Set ("AllowedHelloLoss", UintegerValue (5));
  routing.Set ("RerrRateLimit", UintegerValue (3));
  routing.Set ("TtlIncrement", UintegerValue (4));
  routing.Set ("TtlThreshold", UintegerValue (14));
  stack.SetRoutingHelper (routing); // has effect on the next Install ()
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);
};


void 
RoutingExample::installOnOffApplications(){

  double start_time, stop_time, duration;
  start_time = 10;
  stop_time = 40;

  ApplicationContainer apps [3];
  for (uint32_t i = 0; i < 3; i++)
  {

    if(i == 0)
    {
      server_node = 10;
      client_node = 0;
    }
    else if(i == 1)
    {
      server_node = 10;
      client_node = 1;
    }
    else{
      server_node = 10;
      client_node = 2;
    }
       
    
    OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress(interfaces.GetAddress (server_node), 9)));
    onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));;
    onoff.SetAttribute ("PacketSize", UintegerValue(packet_size));
    onoff.SetAttribute ("DataRate", data_rate);
    
    apps[i] = onoff.Install (nodes.Get(client_node));
    apps[i].Start (Seconds (start_time));
    apps[i].Stop (Seconds (stop_time));

  }

};

void
RoutingExample::printingRoutingTable(){
  Time rtt1 = Seconds(15.0);
  AsciiTraceHelper ascii1;
  Ptr<OutputStreamWrapper> rtw1 = ascii1.CreateFileStream ("xml/demo/routing_table1");
  routing.PrintRoutingTableAllAt(rtt1,rtw1);

  Time rtt2 = Seconds(25);
  AsciiTraceHelper ascii2;
  Ptr<OutputStreamWrapper> rtw2 = ascii2.CreateFileStream ("xml/demo/routing_table2");
  routing.PrintRoutingTableAllAt(rtt2,rtw2);
  
  Time rtt3 = Seconds(35);
  AsciiTraceHelper ascii3;
  Ptr<OutputStreamWrapper> rtw3 = ascii3.CreateFileStream ("xml/demo/routing_table3");
  routing.PrintRoutingTableAllAt(rtt3,rtw3);
};

void
RoutingExample::enablePcapTracing(){
  stack.EnablePcapIpv4All ("xml/demo/pcap/internet"); // gets pcap files of all nodes
}

int
main (int argc, char *argv[])
{
  RoutingExample app_RE;
  app_RE.configuration(argc, argv);
  app_RE.run();
}