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
    // Number of nodes
    uint32_t size = 25;
    // Seed Value
    uint32_t seed = 1;
    // Number of Connections
    uint32_t connections = 5;

  private:
    // parameters
    uint32_t server_node, client_node;
    // Distance between nodes, meters
    //double step = 25;
    // Simulation time, seconds
    double totalTime = 180;
    double duration;
    // Write per-device PCAP traces if true & net-anim file generate
    bool pcap = true;
    bool anim = false;
    // Print routes if true
    bool printRoutes = false;
    //Size of Packet (bytes), Packet interval (Time), Max Packets
    double packet_size = 1024;
    Time packet_interval = MilliSeconds (1000);
    double max_packets = 250;
    StringValue data_rate = StringValue("150kb/s");
    //Internet Stack Helper
    InternetStackHelper stack;
    //Pointer to the packet sink application 
    Ptr<PacketSink> sink[20];
    //The value of the last total received bytes
    uint64_t lastTotalRx = 0;
    //Routing Method
    //AodvHelper routing;
    //OlsrHelper routing;
    DsdvHelper routing;
    // you can configure AODV attributes here using aodv.Set(name, value)

  // network
  // nodes used in the example
  NodeContainer nodes;
  // devices used in the example
  NetDeviceContainer devices;
  // interfaces used in the example
  Ipv4InterfaceContainer interfaces;
  
  private:
    // Create the nodes (i -> quantity)
    void createNodes (int i);
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
    
};

void
RoutingExample::run(){
  
  // Enable logging for UdpClient and
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  createNodes(size);
  createDevices();
  installInternetStack();
  installOnOffApplications();
  if(pcap) enablePcapTracing();
  if(printRoutes) printingRoutingTable();

  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();
  std::string file_path = "xml/flowmonitor/flowmon-";
  file_path += std::to_string(connections);
  file_path += ".xml";

  /*
  FlowMonitorHelper flowmonHelper;
  flowmonHelper.InstallAll ();
  */

  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();

  std::cout << "\n------------------------------Measurement With 'SinkHelper'-------------------------------------\n\n";

  std::cout << " Packet Size: \t\t" << packet_size << " Bytes, " << packet_size / 1024 << " KiloBytes \n";
  std::cout << " Data Rate: \t\t" << data_rate.Get() << "\n\n";
  
  std::cout << "\n--------------------------------------------------------------------------------------------\n";

  flowMonitor->SerializeToXmlFile (file_path, false, true);

  // Define variables to calculate the metrics
  int k=0;
  int totaltxPackets = 0;
  int totalrxPackets = 0;
  int totaltxPacketsR = 0;
  int totalrxPacketsR = 0;
  double totaltxbytes = 0;
  double totalrxbytes = 0;
  double totaltxbytesR = 0;
  double totalrxbytesR = 0;
  double totaldelay = 0;
  double totalrxbitrate = 0;
  double difftx, diffrx;
  double pdf_value, rxbitrate_value, txbitrate_value, delay_value;
  double pdf_total, rxbitrate_total, delay_total;
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
      if ( (!t.destinationAddress.IsSubnetDirectedBroadcast("255.255.255.0")) && (txbitrate_value > 150/1.2) && (rxbitrate_value < 150*1.2)  && txbitrate_value != inf)
      {
          k++;
          std::cout << "\nFlow " << k << " (" << t.sourceAddress << " -> "
          << t.destinationAddress << ")\n";
          //std::cout << "Tx Packets: " << i->second.txPackets << "\n";
          //std::cout << "Rx Packets: " << i->second.rxPackets << "\n";
          //std::cout << "Lost Packets: " << i->second.lostPackets << "\n";
          //std::cout << "Dropped Packets: " << i->second.packetsDropped.size() << "\n";
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
  // Print all nodes statistics
  std::cout << "\nTotal PDF: " << pdf_total << " %\n";
  std::cout << "Total Rx bitrate: " << rxbitrate_total << " kbps\n";
  std::cout << "Total Delay: " << delay_total << " s\n";

  // file pointer 
  std::fstream fout; 
  // opens an existing csv file or creates a new file. 
  fout.open("xml/VanetRCDSDV-report.csv", std::ios::out | std::ios::app);
  fout << connections << "," << seed << "," << pdf_total << "," << rxbitrate_total << "," << delay_total << "\n";

  Simulator::Destroy ();
};

void RoutingExample::configuration(int argc, char ** argv){
  CommandLine cmd;
  cmd.AddValue("size", "Number of nodes", size);
  cmd.AddValue("seed", "Value of seed", seed);
  cmd.AddValue("connections", "Number of connections", connections);
  cmd.Parse (argc, argv);
}

void
RoutingExample::createNodes(int i){
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Mode", StringValue ("Time"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Time", StringValue ("2s"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  //Creating nodes
  nodes.Create(i);

  //Adding Mobility to the created nodes
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
  "MinX", DoubleValue (0.0),
  "MinY", DoubleValue (0.0),
  "DeltaX", DoubleValue (40),
  "DeltaY", DoubleValue (40),
  "GridWidth", UintegerValue (5),
  "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
  "Bounds", RectangleValue (Rectangle (-500,500,-500,500)));

  //mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (nodes);
};

void
RoutingExample::createDevices(){
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();

  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  //wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel","Exponent", StringValue ("2.7"));

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

  //WifiHelper wifi;
  //wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  //wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", 
  //StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (2500));
  devices = wifi.Install (wifiPhy, wifiMac, nodes); 
};

void
RoutingExample::installInternetStack(){
    
  stack.SetRoutingHelper (routing); // has effect on the next Install ()
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);
};

void 
RoutingExample::installOnOffApplications(){

  RngSeedManager::SetSeed(seed);

  double start_time, stop_time, duration;

  Ptr<UniformRandomVariable> a = CreateObject<UniformRandomVariable>();
  a->SetAttribute("Min", DoubleValue (40));
  a->SetAttribute("Max", DoubleValue(totalTime-30));

  Ptr<UniformRandomVariable> rand_nodes = CreateObject<UniformRandomVariable>();
  rand_nodes->SetAttribute("Min", DoubleValue (0));
  rand_nodes->SetAttribute("Max", DoubleValue(size-1));

  ApplicationContainer apps [connections];
  for (uint32_t i = 0; i < connections; i++)
  {

    start_time = a->GetValue();
    Ptr<ExponentialRandomVariable> b = CreateObject<ExponentialRandomVariable>();
    b->SetAttribute("Mean", DoubleValue(30));
    duration = b->GetValue()+1;

    if(duration < 5) duration = 5;

    if ( (start_time + duration) > (totalTime - 10)){
      stop_time = totalTime-10;
    }else{
      stop_time = start_time + duration;
    }

    server_node = rand_nodes->GetInteger (0,size-1);
    // Set random variables of the source (client)
    client_node = rand_nodes->GetInteger (0,size-1);
    // Client and server can not be the same node.
    while (client_node == server_node){
      client_node = rand_nodes->GetInteger (0,size-1);
    }

    // File pointer 
    std::fstream fin;
  
    // Open an existing file 
    fin.open("xml/connections.csv", std::ios::in);

    int rollnum = connections * (seed - 1) + i + 1;

    std::vector<std::string> row;
    std::string line, word;
    while(!fin.eof()){
      
      fin >> line;
      std::stringstream s(line);

      // Satır
      //std::cout << "\n" << line; break;
      
      while(std::getline(s, word, ','))
      {
        row.push_back(word);
      }

      int temp = std::stoi(row[0]);
      if(rollnum == temp)
      {
        client_node = uint32_t(std::stoi(row[3])) - 1;
        server_node = uint32_t(std::stoi(row[4])) - 1;
        std::stringstream start_T(row[5]); start_T >> start_time;
        std::stringstream stop_T(row[6]); stop_T >> stop_time;
        break;
      }
      row.clear();
    }
    fin.close();
    row.clear();
    
    // file pointer 
    std::fstream fout_tests; 
    // opens an existing csv file or creates a new file. 
    fout_tests.open("xml/connectionsDSDVOUT.csv", std::ios::out | std::ios::app);
    uint32_t rownum = connections * (seed - 1) + i + 1;  
    // write csv
    fout_tests << rownum << "," << connections << "," << seed << "," << client_node + 1 << "," << server_node + 1 << "," << start_time << "," << stop_time << "\n";
    
    std::cout << "\n Packet Flow: \t\t" << client_node + 1 << " to " << server_node + 1;
    std::cout << "\n Start_time: \t\t" << start_time << "s";
    std::cout << "\n Stop_time: \t\t" << stop_time << "s\n";

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
  Ptr<OutputStreamWrapper> rtw1 = ascii1.CreateFileStream ("xml/routing_table1");
  routing.PrintRoutingTableAllAt(rtt1,rtw1);

  Time rtt2 = Seconds(16.0);
  AsciiTraceHelper ascii2;
  Ptr<OutputStreamWrapper> rtw2 = ascii2.CreateFileStream ("xml/routing_table2");
  routing.PrintRoutingTableAllAt(rtt2,rtw2);
  
  Time rtt3 = Seconds(30.0);
  AsciiTraceHelper ascii3;
  Ptr<OutputStreamWrapper> rtw3 = ascii3.CreateFileStream ("xml/routing_table3");
  routing.PrintRoutingTableAllAt(rtt3,rtw3);
};

void
RoutingExample::enablePcapTracing(){
  stack.EnablePcapIpv4All ("xml/pcap/internet"); // gets pcap files of all nodes
}

/*
void
RoutingExample::calculateThroughput(){
  
  Time now = Simulator::Now ();                                               // Return the simulator's virtual time.
  double cur = (sink[0]->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;     // Convert Application RX Packets to MBits.
  std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink[0]->GetTotalRx ();
  //Simulator::Schedule (MilliSeconds (100), &RoutingExample::calculateThroughput);
  
}
*/

int
main (int argc, char *argv[])
{
  /*
  uint8_t a = 3;
  a |= (1<<2);
  a += 1;
  a &= (1);
  std::cout << "\n" << bool(a == 0) << "\n";
  return 0;
  */
  RoutingExample app_RE;
  app_RE.configuration(argc, argv);
  app_RE.run();
}