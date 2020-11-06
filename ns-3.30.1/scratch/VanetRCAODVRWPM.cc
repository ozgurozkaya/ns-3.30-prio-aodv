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
#include "ns3/stats-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("VanetRC");

void changePosition (Ptr<Node> node, Vector position){
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  mobility->SetPosition(position);
}

void
CalculateThroughput (){
  Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
  //double cur = (sink->GetTotalRx() - lastTotalRx) / 1024;     /* Convert Application RX Packets to MBits. */
  //std::cout << now.GetSeconds () << "s: \t" << cur << " packets" << " To: " << sink->GetNode()->GetId() << std::endl;
  //lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (1000), &CalculateThroughput);
}

double
DelayOutput (std::string key, std::string variable, const StatisticalSummary *statSum){
  return statSum->getMean();
}

void
TxCallback (Ptr<CounterCalculator<uint32_t> > datac, std::string path, Ptr<const Packet> packet) {
  
  NS_LOG_INFO ("Sent frame counted in " << datac->GetKey ());
  datac->Update ();
  //std::cout << "TXCALLBACK - \t Simulator::NOW (): " << Simulator::Now() << "\n";
  // end TxCallback
}

void
RxCallback (Ptr<CounterCalculator<uint32_t> > datac, std::string path, Ptr<const Packet> packet, const Address &) {
  
  NS_LOG_INFO ("Sent frame counted in " << datac->GetKey ());
  datac->Update ();
  TimestampTag timestamp;
  packet->FindFirstMatchingByteTag (timestamp);
  Time tx = timestamp.GetTimestamp ();
  //std::cout << "RXCALLBACK - \t Simulator::NOW (): " << Simulator::Now() << "  |  Timestamp.GetTimestamp (): " << tx << "   |   Diff: " << (Simulator::Now () - tx) << "\n";
  
  // end TxCallback
}

void
RxDelayCallback (Ptr<TimeMinMaxAvgTotalCalculator> delaystatc, std::string path, Ptr<const Packet> packet, const Address &) {
  
  NS_LOG_INFO ("Sent frame counted in " << delaystatc->GetKey ());
  
  TimestampTag timestamp;
  packet->FindFirstMatchingByteTag (timestamp);
  Time tx = timestamp.GetTimestamp ();
  delaystatc->Update ( (Simulator::Now () - tx) );
  //std::cout << "RXDELAYCB - \t Simulator::NOW (): " << Simulator::Now() << "  |  Timestamp.GetTimestamp (): " << tx << "   |   Diff: " << (Simulator::Now () - tx) << "\n";
  // end TxCallback
}


class RoutingExample{
  public:
    void run();
    // argc & argv configuration
    void configuration(int argc, char ** argv);
    // Number of nodes
    uint32_t size = 50;
    uint32_t meters = 1000;
    // Seed Value
    uint32_t seed = 1001;
    // Number of Connections
    uint32_t connections = 10;
    // Seconds of Connections
    double total_connection_time = 0;
    Ptr<PacketCounterCalculator> totalRx[1000];
    Ptr<CounterCalculator<uint32_t> > totalTx[1000];
    Ptr<TimeMinMaxAvgTotalCalculator> delayStat[1000];
    DataCollector data;

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
    // Print routes if true
    bool printRoutes = false;
    //Size of Packet (bytes), Packet interval (Time), Max Packets
    double packet_size = 512;
    //Time packet_interval = MilliSeconds (1000);
    //double max_packets = 250;
    StringValue data_rate = StringValue("5Kb/s");
    double data_rate_2 = 5;
    //Internet Stack Helper
    InternetStackHelper stack;
    //Pointer to the packet sink application 
    Ptr<PacketSink> sink[1000];
    //The value of the last total received bytes
    uint64_t lastTotalRx = 0;
    //Routing Method
    AodvHelper routing;
    //OlsrHelper routing;
    //DsdvHelper routing;
    DsrHelper dsrrouting;
    DsrMainHelper dsrMain;
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
    
};

void
RoutingExample::run(){
  RngSeedManager::SetSeed(seed);

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

  std::string animFile = "xml/animation-AODV.xml";
  AnimationInterface animation (animFile);
  //animation.SetMaxPktsPerTraceFile(500000);

  //Simulator::Schedule(Seconds(20), &changePosition, nodes.Get(0), Vector(500,500,0));
  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();

  std::cout << "\n------------------------------Measurement With 'SinkHelper'-------------------------------------\n\n";

  std::cout << " Packet Size: \t\t" << packet_size << " Bytes, " << packet_size / 1024 << " KiloBytes \n";
  std::cout << " Data Rate: \t\t" << data_rate.Get() << "\n\n";
  /*
  for (uint32_t i = 0; i < connections; i++)
  {
    std::cout << " Packets Received: \t" << sink[i]->GetTotalRx() / double(packet_size) << "\n";
    std::cout << " Bytes Received: \t" << sink[i]->GetTotalRx() << "\n";
    std::cout << " Throughput: \t\t" << sink[i]->GetTotalRx() / 1024 / 20 << " KiloBytes/sec \n\n";
  }
  */
  std::cout << "\n--------------------------------------------------------------------------------------------\n";

  //---------------------------------------------------------------
  //-- Flowmonitor START
  //---------------------------------------------------------------

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
      if ( (!t.destinationAddress.IsSubnetDirectedBroadcast("255.255.255.0")) && (txbitrate_value > 5/1.2) && (rxbitrate_value < 5*1.2)  && txbitrate_value != inf)
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
          std::cout << "Total Delay: " << (double) i->second.delaySum.GetSeconds() << "s\n";
          //std::cout << "Rx bitrate: " << rxbitrate_value << " kbps\n";
          //std::cout << "Tx bitrate: " << txbitrate_value << " kbps\n";
          std::cout << "Tx packets: " << i->second.txPackets << " packets\n";
          std::cout << "Rx packets: " << i->second.rxPackets << " packets\n\n";
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
  //std::cout << "Total Rx bitrate: " << rxbitrate_total << " kbps\n";
  std::cout << "Total Rx Packets: " << totalrxPackets << "\n";
  std::cout << "Total Delay: " << totaldelay << " s\n";
  std::cout << "Avg Delay: " << delay_total << " s\n";

  // file pointer 
  std::fstream fout; 
  // opens an existing csv file or creates a new file. 
  fout.open("xml/VanetRCAODV-report-flowmonitor.csv", std::ios::out | std::ios::app);
  fout << connections << "," << seed << "," << pdf_total << "," << rxbitrate_total << "," << delay_total << "\n";
  fout.close();
  
  //---------------------------------------------------------------
  //-- Flowmonitor START
  //---------------------------------------------------------------

  //---------------------------------------------------------------
  //-- StatsModule Output
  //---------------------------------------------------------------
  Ptr<DataOutputInterface> output;
  NS_LOG_INFO ("Creating omnet formatted data output.");
  output = CreateObject<OmnetDataOutput>();
  output->Output(data);

  data_rate_2 = data_rate_2 * 1000; // turning Kb to bits
  double pdf_total_statsModule = delayStat[0]->GetTotal() / floor(total_connection_time * (data_rate_2 / (packet_size * 8))) * 100;
  std::cout << "\n" << total_connection_time << "\n" << pdf_total_statsModule << "\n";
  
  // file pointer 
  std::fstream fout2; 
  // opens an existing csv file or creates a new file. 
  fout2.open("xml/VanetRCAODV-report-statsModule.csv", std::ios::out | std::ios::app);
  fout2 << connections << "," << seed << "," << pdf_total_statsModule << "," << "0" << "," << delayStat[0]->GetAverage().GetSeconds() << "\n";

  Simulator::Destroy ();
};

void RoutingExample::configuration(int argc, char ** argv){
  CommandLine cmd;
  cmd.AddValue("size", "Number of nodes", size);
  cmd.AddValue("seed", "Value of seed", seed);
  cmd.AddValue("connections", "Number of connections", connections);
  cmd.AddValue("meters", "One side of square as meters", meters);
  cmd.Parse (argc, argv);
}

void
RoutingExample::createNodes(int i){
  //Creating nodes
  nodes.Create(i);
  
  //Adding Mobility to the created nodes
  MobilityHelper mobility;
  int64_t streamIndex = 0; // used to get consistent mobility across scenarios
  
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max="+to_string(meters)+"]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max="+to_string(meters)+"]"));

  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  streamIndex += taPositionAlloc->AssignStreams (streamIndex);
  
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                              "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=15.0]"),
                              "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=30]"),
                              "PositionAllocator", PointerValue (taPositionAlloc));

  mobility.SetPositionAllocator (taPositionAlloc);
  
  /*
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                               "MinX", DoubleValue (0.0),
                               "MinY", DoubleValue (0.0),
                               "DeltaX", DoubleValue (250),
                               "DeltaY", DoubleValue (250),
                               "GridWidth", UintegerValue (10),
                               "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  */
  mobility.Install (nodes);

  streamIndex += mobility.AssignStreams (nodes, streamIndex);
  NS_UNUSED (streamIndex); // From this point, streamIndex is unused
};

void
RoutingExample::createDevices(){
  std::string phyMode ("OfdmRate12Mbps");
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();

  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  //wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel");
  //wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel","Exponent", StringValue ("2.7"));
  
  wifiPhy.Set ("TxPowerLevels", UintegerValue (1) );
  wifiPhy.Set ("TxPowerEnd", DoubleValue (36.66) );
  wifiPhy.Set ("TxPowerStart", DoubleValue (36.66) );
  
  wifiPhy.SetChannel (wifiChannel.Create ());
  
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",StringValue (phyMode), "ControlMode",StringValue (phyMode));

  devices = wifi.Install (wifiPhy, wifiMac, nodes); 
};

void
RoutingExample::installInternetStack(){

  //--------------------------------------------
  //--AODV Routing Protocol
  //--------------------------------------------
  //routing.Set ("AllowedHelloLoss", UintegerValue (5));
  //routing.Set ("RerrRateLimit", UintegerValue (3));
  //routing.Set ("TtlIncrement", UintegerValue (4));
  //routing.Set ("TtlThreshold", UintegerValue (14));
  //routing.Set ("BlackListTimeout", TimeValue ( Seconds (0)));
  stack.SetRoutingHelper (routing); // has effect on the next Install ()
  stack.Install (nodes);
  //--------------------------------------------
  //--AODV Routing Protocol
  //--------------------------------------------

  //--------------------------------------------
  //--DSR Routing Protocol
  //--------------------------------------------
  /*
  stack.Install (nodes);
  dsrMain.Install (dsrrouting, nodes);
  */
  //--------------------------------------------
  //--DSR Routing Protocol
  //--------------------------------------------

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);
};

void 
RoutingExample::installOnOffApplications(){

  //--------------------------------------
  //-- Stats Module START
  //--------------------------------------

  //Adding sinks to all nodes
  ApplicationContainer apps_sink[nodes.GetN()];

  for(uint32_t tmpi = 0; tmpi < nodes.GetN(); tmpi++){
      PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
      apps_sink[tmpi] = sinkHelper.Install (nodes.Get (tmpi));
      //sink = StaticCast<PacketSink> (apps_sink[tmpi].Get(0));
      apps_sink[tmpi].Start (Seconds (0));
      apps_sink[tmpi].Stop (Seconds (180));
    }

  string experiment ("DSR-routing-test");
  string strategy ("wifi-default");
  string input;
  string runID ("AODV");

  // Create a DataCollector object to hold information about this run.
  data.DescribeRun (experiment, strategy, input, runID);

  // Add any information we wish to record about this run.
  data.AddMetadata ("author", "Özgür");

  //--------------------------------------
  //-- Stats Module END
  //--------------------------------------

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

    start_time = a->GetInteger();
    Ptr<ExponentialRandomVariable> b = CreateObject<ExponentialRandomVariable>();
    b->SetAttribute("Mean", DoubleValue(30));
    duration = b->GetInteger()+1;

    if(duration < 20) duration = 20;
    
    if(start_time > 150) start_time = 150;

    if ( (start_time + duration) > (totalTime - 10)){
      stop_time = totalTime-10;
      duration = stop_time - start_time;
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

   /* 
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

      //std::cout << "\n" << line; break;
      
      while(std::getline(s, word, ','))
      {
        row.push_back(word);
      }

      int temp = std::stoi(row[0]);
      if(rollnum == temp)
      {
        client_node = uint32_t(std::stoi(row[3]));
        server_node = uint32_t(std::stoi(row[4]));
        std::stringstream start_T(row[5]); start_T >> start_time;
        int start_time = int(start_time);
        std::stringstream stop_T(row[6]); stop_T >> stop_time;
        int stop_time = int(stop_time);
        break;
      }
      row.clear();
    }
    fin.close();
    row.clear();
    */
    
    duration = stop_time - start_time;
    total_connection_time += duration;
    
    // file pointer 
    std::fstream fout_tests;
    // opens an existing csv file or creates a new file. 
    fout_tests.open("xml/connectionsVanetRC-AODV-OUT-330.csv", std::ios::out | std::ios::app);
    uint32_t rownum = connections * (seed - 1) + i + 1;  
    // write csv
    fout_tests << rownum << "," << connections << "," << seed << "," << client_node << "," << server_node << "," << start_time << "," << stop_time << "\n";

    std::cout << "\n Packet Flow: \t\t" << client_node << " to " << server_node;
    std::cout << "\n Start_time: \t\t" << start_time << "s";
    std::cout << "\n Stop_time: \t\t" << stop_time << "s";
    std::cout << "\n Duration: \t\t" << duration << "s\n";

    OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress(interfaces.GetAddress (server_node), 9)));
    onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));;
    onoff.SetAttribute ("PacketSize", UintegerValue(packet_size));
    onoff.SetAttribute ("DataRate", data_rate);
    
    apps[i] = onoff.Install (nodes.Get(client_node));
    apps[i].Start (Seconds (start_time));
    apps[i].Stop (Seconds (stop_time));

    // -------------------------------------
    // Tx Packet Tracker
    // -------------------------------------
    
    totalTx[i] = CreateObject<CounterCalculator<uint32_t> >();
    totalTx[i]->SetKey ("DSR-appTx-packets");
    totalTx[i]->SetContext ("node-"+to_string(client_node));
    Config::Connect ("/NodeList/" + to_string(client_node) + "/ApplicationList/*/$ns3::OnOffApplication/Tx", 
                      MakeBoundCallback (&TxCallback, totalTx[i]));
    data.AddDataCalculator (totalTx[i]);

    // -------------------------------------
    // Rx packet Tracker
    // -------------------------------------

    totalRx[i] = CreateObject<PacketCounterCalculator>();
    totalRx[i]->SetKey ("DSR-rx-packets");
    totalRx[i]->SetContext ("node-"+to_string(server_node));
    Config::Connect ("/NodeList/"+to_string(server_node)+"/ApplicationList/*/$ns3::PacketSink/Rx",
                      MakeBoundCallback (&RxCallback, totalRx[i]));
    data.AddDataCalculator (totalRx[i]);

  }

    // -------------------------------------
    // Delay Tracker
    // -------------------------------------
    
    delayStat[0] = CreateObject<TimeMinMaxAvgTotalCalculator>();
    delayStat[0]->SetKey ("delay");
    delayStat[0]->SetContext ("*");
    Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeBoundCallback (&RxDelayCallback, delayStat[0]));
    data.AddDataCalculator (delayStat[0]);

};

void
RoutingExample::printingRoutingTable(){
  
  Time rtt1 = Seconds(75.0);
  AsciiTraceHelper ascii1;
  Ptr<OutputStreamWrapper> rtw1 = ascii1.CreateFileStream ("xml/routing_table1");
  routing.PrintRoutingTableAllAt(rtt1,rtw1);

  Time rtt2 = Seconds(80.0);
  AsciiTraceHelper ascii2;
  Ptr<OutputStreamWrapper> rtw2 = ascii2.CreateFileStream ("xml/routing_table2");
  routing.PrintRoutingTableAllAt(rtt2,rtw2);
  
  Time rtt3 = Seconds(95.0);
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
  RoutingExample app_RE;
  app_RE.configuration(argc, argv);
  app_RE.run();
}
