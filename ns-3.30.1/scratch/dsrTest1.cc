/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/dsr-module.h"
#include "ns3/aodv-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/stats-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("DSRTEST1");


//----------------------------------------------------------------------
//-- TimestampTag START
//------------------------------------------------------
/*
class TimestampTag : public Tag {
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);

  // these are our accessors to our tag structure
  void SetTimestamp (Time time);
  Time GetTimestamp (void) const;

  void Print (std::ostream &os) const;

private:
  Time m_timestamp;

  // end class TimestampTag
};

TypeId 
TimestampTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("TimestampTag")
    .SetParent<Tag> ()
    .AddConstructor<TimestampTag> ()
    .AddAttribute ("Timestamp",
                   "Some momentous point in time!",
                   EmptyAttributeValue (),
                   MakeTimeAccessor (&TimestampTag::GetTimestamp),
                   MakeTimeChecker ())
  ;
  return tid;
}
TypeId 
TimestampTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t 
TimestampTag::GetSerializedSize (void) const
{
  return 8;
}
void 
TimestampTag::Serialize (TagBuffer i) const
{
  int64_t t = m_timestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&t, 8);
}
void 
TimestampTag::Deserialize (TagBuffer i)
{
  int64_t t;
  i.Read ((uint8_t *)&t, 8);
  m_timestamp = NanoSeconds (t);
}

void
TimestampTag::SetTimestamp (Time time)
{
  m_timestamp = time;
}
Time
TimestampTag::GetTimestamp (void) const
{
  return m_timestamp;
}

void 
TimestampTag::Print (std::ostream &os) const
{
  os << "t=" << m_timestamp;
}
*/
//----------------------------------------------------------------------
//-- TimestampTag END
//------------------------------------------------------


Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */

void
CalculateThroughput ()
{
  Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
  //double cur = (sink->GetTotalRx() - lastTotalRx) / 1024;     /* Convert Application RX Packets to MBits. */
  //std::cout << now.GetSeconds () << "s: \t" << cur << " packets" << " To: " << sink->GetNode()->GetId() << std::endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (1000), &CalculateThroughput);
}

void TxCallback (Ptr<CounterCalculator<uint32_t> > datac, std::string path, Ptr<const Packet> packet) {
  
  NS_LOG_INFO ("Sent frame counted in " << datac->GetKey ());
  datac->Update ();
  std::cout << "\t\t--------\n";
  std::cout << "TXCALLBACK- \t Simulator::NOW (): " << Simulator::Now() << "\n";
  // end TxCallback
}

void RxCallback (Ptr<CounterCalculator<uint32_t> > datac, std::string path, Ptr<const Packet> packet, const Address &) {
  
  NS_LOG_INFO ("Sent frame counted in " << datac->GetKey ());
  datac->Update ();
  //TimestampTag timestamp;
  //packet->FindFirstMatchingByteTag (timestamp);
  //std::cout << "RXCALLBACK- \t Simulator::NOW (): " << Simulator::Now().GetTimeStep() << "  |  timestamp.GetTimestamp (): " << timestamp.GetTimestamp() << "   |   Diff: " << (Simulator::Now () - timestamp.GetTimestamp()) << "\n";
  std::cout << "RXCALLBACK- \t Simulator::NOW (): " << Simulator::Now() << "\n";
  
  // end TxCallback
}

void RxDelayCallback (Ptr<TimeMinMaxAvgTotalCalculator> delaystatc, std::string path, Ptr<const Packet> packet, const Address &) {
  
  NS_LOG_INFO ("Sent frame counted in " << delaystatc->GetKey ());
  //TimestampTag timestamp;
  //packet->FindFirstMatchingByteTag (timestamp);
  //Time tx = timestamp.GetTimestamp ();
  //delaystatc->Update ( (Simulator::Now () - tx) );
  //std::cout << "RXDELAYCB - \t Simulator::NOW (): " << Simulator::Now() << "  |  timestamp.GetTimestamp (): " << tx << "   |   Diff: " << (Simulator::Now () - tx) << "\n";
  //std::cout << "RXDELAYCB - \t Simulator::NOW (): " << Simulator::Now() << "\n";
  // end TxCallback
  std::cout << "\t\t--------\n";
}

int 
main (int argc, char *argv[])
{
    //NS_LOG_UNCOND ("Uncond print, DSR TEST");
    //LogComponentEnable ("OnOffApplication", LOG_LEVEL_ALL);
    //LogComponentEnable ("UdpL4Protocol", LOG_LEVEL_ALL);
    //LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);

    string experiment ("DSR-routing-test");
    string strategy ("wifi-default");
    string input;
    string runID;

    //--------------------------------------
    //-- Creating nodes & mobility
    //--------------------------------------
    NodeContainer nodes;
    nodes.Create(5);

    Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Mode", StringValue ("Time"));
    Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Time", StringValue ("2s"));
    Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Speed", StringValue ("ns3::ConstantRandomVariable[Constant=10.0]"));
    
    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
    "MinX", DoubleValue (0.0),
    "MinY", DoubleValue (0.0),
    "DeltaX", DoubleValue (52),
    "DeltaY", DoubleValue (52),
    "GridWidth", UintegerValue (5),
    "LayoutType", StringValue ("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    //mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (-500,500,-500,500)));
    mobility.Install(nodes);

    //--------------------------------------
    //-- Creating devices & channel
    //--------------------------------------
    NetDeviceContainer devices;
    WifiHelper wifi;
    WifiMacHelper wifiMac;
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    wifiPhy.SetChannel (wifiChannel.Create ());
    wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
    devices = wifi.Install (wifiPhy, wifiMac, nodes); 

    //--------------------------------------
    //-- Creating Routing & Internet
    //--------------------------------------
    InternetStackHelper internet;
    Ipv4InterfaceContainer interfaces;
    /*
    DsrHelper routing;
    DsrMainHelper dsrMain;
    internet.Install (nodes);
    dsrMain.Install (routing, nodes);
    */
    AodvHelper routing;
    internet.SetRoutingHelper (routing);
    internet.Install (nodes);
    
    Ipv4AddressHelper address;
    address.SetBase ("10.0.0.0", "255.255.255.0");
    interfaces = address.Assign (devices);

    //--------------------------------------
    //-- Creating Flow & Stats Module
    //--------------------------------------

    ApplicationContainer apps_sink[nodes.GetN()];

    // Creating Sinks for all nodes
    for(uint32_t tmpi = 0; tmpi < nodes.GetN(); tmpi++){
      PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
      apps_sink[tmpi] = sinkHelper.Install (nodes.Get (tmpi));
      sink = StaticCast<PacketSink> (apps_sink[tmpi].Get(0));
      apps_sink[tmpi].Start (Seconds (1));
      apps_sink[tmpi].Stop (Seconds (9));
    }

    // Creating On-Off Applications and Stats module
    ApplicationContainer connections[5];
    for(int tmpi = 0 ; tmpi < 1; tmpi++){
      OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress(interfaces.GetAddress (3), uint16_t(9))));
      onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
      onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
      onoff.SetAttribute ("PacketSize", UintegerValue (1024));
      onoff.SetAttribute ("DataRate", DataRateValue (DataRate ("1Mbps")));
      //onoff.SetAttribute ("EnableSeqTsSizeHeader", BooleanValue(true));
      connections[tmpi] = onoff.Install (nodes.Get(4));

      connections[tmpi].Start (Seconds (2));
      Simulator::Schedule (Seconds (2.1), &CalculateThroughput);
      connections[tmpi].Stop (Seconds (8));
    }

    //--------------------------------------
    //-- Stats Module
    //--------------------------------------
  
    // Create a DataCollector object to hold information about this run.
    DataCollector data;
    data.DescribeRun (experiment, strategy, input, runID);

    // Add any information we wish to record about this run.
    data.AddMetadata ("author", "Özgür");

    // -------------------------------------
    // Tx Packet Tracker
    // -------------------------------------
    Ptr<CounterCalculator<uint32_t> > totalTx = CreateObject<CounterCalculator<uint32_t> >();
    totalTx->SetKey ("DSR-appTx-packets");
    totalTx->SetContext ("node[4]");
    Config::Connect ("/NodeList/4/ApplicationList/*/$ns3::OnOffApplication/Tx", MakeBoundCallback (&TxCallback, totalTx));
    data.AddDataCalculator (totalTx);

    // -------------------------------------
    // Rx packet Tracker
    // -------------------------------------
    Ptr<PacketCounterCalculator> totalRx[4];
    totalRx[0] = CreateObject<PacketCounterCalculator>();
    totalRx[0]->SetKey ("DSR-rx-packets");
    totalRx[0]->SetContext ("node[0]");
    Config::Connect ("/NodeList/0/ApplicationList/*/$ns3::PacketSink/Rx", MakeBoundCallback (&RxCallback, totalRx[0]));
    data.AddDataCalculator (totalRx[0]);

    totalRx[1] = CreateObject<PacketCounterCalculator>();
    totalRx[1]->SetKey ("DSR-rx-packets");
    totalRx[1]->SetContext ("node[1]");
    Config::Connect ("/NodeList/1/ApplicationList/*/$ns3::PacketSink/Rx", MakeBoundCallback (&RxCallback, totalRx[1]));
    data.AddDataCalculator (totalRx[1]);

    totalRx[2] = CreateObject<PacketCounterCalculator>();
    totalRx[2]->SetKey ("DSR-rx-packets");
    totalRx[2]->SetContext ("node[2]");
    Config::Connect ("/NodeList/2/ApplicationList/*/$ns3::PacketSink/Rx", MakeBoundCallback (&RxCallback, totalRx[2]));
    data.AddDataCalculator (totalRx[2]);

    totalRx[3] = CreateObject<PacketCounterCalculator>();
    totalRx[3]->SetKey ("DSR-rx-packets");
    totalRx[3]->SetContext ("node[3]");
    Config::Connect ("/NodeList/3/ApplicationList/*/$ns3::PacketSink/Rx", MakeBoundCallback (&RxCallback, totalRx[3]));
    data.AddDataCalculator (totalRx[3]);

    // -------------------------------------
    // Delay Tracker
    // -------------------------------------
    Ptr<TimeMinMaxAvgTotalCalculator> delayStat = CreateObject<TimeMinMaxAvgTotalCalculator>();
    delayStat->SetKey ("delay");
    delayStat->SetContext (".");
    Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeBoundCallback (&RxDelayCallback, delayStat));
    data.AddDataCalculator (delayStat);

    // -------------------------------------
    //PCAP
    // -------------------------------------
    internet.EnablePcapIpv4All ("xml/dsrtest");

    Simulator::Stop (Seconds (10));
    Simulator::Run ();

    Ptr<DataOutputInterface> output;
    NS_LOG_INFO ("Creating omnet formatted data output.");
    output = CreateObject<OmnetDataOutput>();
    output->Output(data);
    
    Simulator::Destroy ();

    std::cout << "\n--- " << double(sink->GetTotalRx()) / 1024 << " KBytes ---\n";
    std::cout << "\n--- " << totalRx[1]->GetCount() << " ---\n";
}



