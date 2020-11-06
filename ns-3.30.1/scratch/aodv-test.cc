/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */


#include <iostream>
#include <cmath>
#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/onoff-application.h"
#include "ns3/netanim-module.h"
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/applications-module.h"

using namespace ns3;

void changePosition (Ptr<Node> node, Vector position){
	Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
	mobility->SetPosition(position);
}

int main (int argc, char **argv)
{
	//-------------------------------------
	//Creating Nodes & Mobility
	//-------------------------------------
	NodeContainer nodes;
	nodes.Create (20);

	MobilityHelper mobility;
	
	//-------------------------------------
	//Grid Position Allocator & Constant Position Mobility Model
	//-------------------------------------
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
	                             "MinX", DoubleValue (0.0),
	                             "MinY", DoubleValue (0.0),
	                             "DeltaX", DoubleValue (50),
	                             "DeltaY", DoubleValue (50),
	                             "GridWidth", UintegerValue (10),
	                             "LayoutType", StringValue ("RowFirst"));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	
	/*
	//-------------------------------------
	//Random Ractangle Position Allocator & Random Waypoint Mobility Model
	//-------------------------------------
	ObjectFactory pos;
	pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
	pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
	pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));
	Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
	mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
	                          "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"),
	                          "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=20]"),
	                          "PositionAllocator", PointerValue (taPositionAlloc));
	mobility.SetPositionAllocator (taPositionAlloc);
	*/
	mobility.Install (nodes);
	

	//-------------------------------------
	//Creating Devices
	//-------------------------------------
	NetDeviceContainer devices;
	WifiMacHelper wifiMac;
	wifiMac.SetType ("ns3::AdhocWifiMac");
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	wifiPhy.SetChannel (wifiChannel.Create ());
	WifiHelper wifi;
  	wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",StringValue ("DsssRate11Mbps"), "ControlMode",StringValue ("DsssRate11Mbps"));
	devices = wifi.Install (wifiPhy, wifiMac, nodes); 
	//Enable Pcap files
	wifiPhy.EnablePcapAll (std::string ("aodv"));

	//-------------------------------------
	//Creating Internet Stack
	//-------------------------------------
	Ipv4InterfaceContainer interfaces;
	AodvHelper aodv;
	InternetStackHelper stack;
	stack.SetRoutingHelper (aodv);
	stack.Install (nodes);
	Ipv4AddressHelper address;
	address.SetBase ("10.0.0.0", "255.0.0.0");
	interfaces = address.Assign (devices);

	//-------------------------------------
	//Installing Application (OnOff Application)
	//-------------------------------------
	//Adding sinks to all nodes
	ApplicationContainer apps_sink[nodes.GetN()];
	ApplicationContainer apps[1];
	for(uint32_t tmpi = 0; tmpi < nodes.GetN(); tmpi++){
	  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
	  apps_sink[tmpi] = sinkHelper.Install (nodes.Get (tmpi));
	  apps_sink[tmpi].Start (Seconds (0));
	  apps_sink[tmpi].Stop (Seconds (75));
	}
	//OnnOff Application 
    OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress(interfaces.GetAddress (19), 9)));
    onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));;
    onoff.SetAttribute ("DataRate", StringValue("50Kb/s"));
    apps[0] = onoff.Install (nodes.Get(11));
    apps[0].Start (Seconds (25));
    apps[0].Stop (Seconds (75));

	//-------------------------------------
	//Getting Animation Xml for NetAnim
	//-------------------------------------
	std::string animFile = "aodv-test-animation.xml";
	AnimationInterface animation (animFile);
	animation.SetMaxPktsPerTraceFile(500000);

	Simulator::Schedule(Seconds(35), &changePosition, nodes.Get(3), Vector(0,200,0));
	Simulator::Schedule(Seconds(45), &changePosition, nodes.Get(15), Vector(0,200,0));
	Simulator::Schedule(Seconds(55), &changePosition, nodes.Get(7), Vector(0,200,0));
	Simulator::Stop (Seconds (75));
	Simulator::Run ();
	Simulator::Destroy();

}