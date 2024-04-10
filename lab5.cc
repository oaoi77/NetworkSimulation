#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/netanim-module.h"
#include <cstdio>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Final Project of Group 7");

int main(int argc, char *argv[]){
    uint32_t packetSize = 512; 
    uint32_t simTime = 10; //simulaton time in second 
    uint32_t maxPacket = 100; 
    uint32_t interval = 1; 
    uint32_t serverNode = 0; //set node 0 to the server node 
    bool verbose = false;
    bool pcap = false;

    CommandLine cmd;
    cmd.AddValue("packetSize", "size of the packet", packetSize);
    cmd.AddValue("maxPackets", "Max packets to send", maxPacket);
    cmd.AddValue("interval", "Interval between packets", interval);
    cmd.AddValue("verbose", "Enable logging", verbose);
    cmd.AddValue("pcap", "Enable pcap", pcap);
    cmd.Parse(argc, argv);


    for (uint32_t i = 2; i <= 30; i++){
        if (verbose){
            LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
            LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
        
        }
        NodeContainer nodes;
        nodes.Create(i);          // Creating node

        // Create channnel helper and PHY helper
        YansWifiChannelHelper chan = YansWifiChannelHelper::Default();
        YansWifiPhyHelper phy;

        phy.SetChannel(chan.Create());

        // Initialize Mac helper
        WifiMacHelper mac;

        // Initialize WifiHelper, defalut: 802.11ax
        WifiHelper wifi;

        mac.SetType("ns3::AdhocWifiMac");

        NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

        MobilityHelper mobility;
        mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                        "MinX", DoubleValue(0.0),
                                        "MinY", DoubleValue(0.0),
                                        "DeltaX", DoubleValue(10.0),
                                        "DeltaY", DoubleValue(10.0),
                                        "GridWidth", UintegerValue(5),
                                        "LayoutType", StringValue("RowFirst"));

        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(nodes);

        InternetStackHelper stack;
        stack.Install(nodes);

        Ipv4AddressHelper address;
        address.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer nodeInterfaces;
        nodeInterfaces = address.Assign(devices);

        UdpEchoServerHelper echoServer(9);

        ApplicationContainer serverApps = echoServer.Install(nodes.Get(serverNode));
        serverApps.Start(Seconds(0.0));
        serverApps.Stop(Seconds(simTime));

        UdpEchoClientHelper echoClient(nodeInterfaces.GetAddress(serverNode), 9);
        echoClient.SetAttribute("MaxPackets", UintegerValue(maxPacket));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(interval)));
        echoClient.SetAttribute("PacketSize", UintegerValue(packetSize));
        
        // Exclude servernode
        for (uint32_t j = 0; j < i; j++){
            if (j == serverNode) continue;
            ApplicationContainer clientApp = echoClient.Install(nodes.Get(j));
            clientApp.Start(Seconds(0.0));
            clientApp.Stop(Seconds(simTime));
        }
        
        if (pcap){
            phy.EnablePcap("final", devices.Get(2), true);
        }

        // Monitoring for collecting data
        Ptr<FlowMonitor> flowMonitor;
        FlowMonitorHelper flowHelper;
        flowMonitor = flowHelper.InstallAll();

        // Visualizing by NetAnim
        char animfile[100];
        sprintf(animfile, "animation-for-%d-nodes.xml", i);
        AnimationInterface anim(animfile);
        Simulator::Stop(Seconds(simTime));
        Simulator::Run();

        char filename[100];
        sprintf(filename, "analyzed-%d-nodes.xml", i);
        flowMonitor->SerializeToXmlFile(filename, true, true);

        Simulator::Destroy();
    }
}




