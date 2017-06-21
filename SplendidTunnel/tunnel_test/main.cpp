//
//  main.cpp
//  tunnel_test
//
//  Created by CmST0us on 17/6/5.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#include <iostream>
#include <future>
#include <utility>
#include "SLClient.hpp"
#include "TunnelMessage.pb.h"
#include <event2/thread.h>
using namespace std;


int main(int argc, const char * argv[]) {
    
    SLClient client;
    bool isEstablish = false;
    client.addOptions("username", "admin")
          .addOptions("password", "passwd");
    
    auto estcb = [&](SLTunnel::TunnelStatus status, st::TunnelMessage &msg, void* ctx) {
        cout<<"Tunnel Establish"<<endl;
        isEstablish = true;
        
    };
    
    auto errcb = [](SLTunnel::TunnelStatus status, void *ctx) {
        cout<<"Tunnel Error"<<endl;
        
    };
    auto datacb = [](SLTunnel::TunnelStatus status, st::TunnelMessage &msg, void *ctx) {
//        cout<<msg.packet().packetdata()<<endl;
    };
    
    client.set_mEstablishCallback(estcb).set_mErrorCallback(errcb).set_mReadPacketCallback(datacb);
    
    if (client.startTunnelWithServerAddress("127.0.0.1", 10080) == SLTunnel::OnEstablish) {
        cout<<"Tunnel On Establish"<<endl;
        auto f = std::async(std::launch::async, [&](){
            return client.dispatchEvent();
        });
        
        while (1) {
            
            unique_ptr<unsigned char> str(new unsigned char[1024]);
            memset(str.get(), 1, 1024);
            if (!isEstablish) {
                continue;
            }
            st::TunnelMessage dataMsg;
            dataMsg.set_type(st::TunnelMessage_Type_REQUEST);
            dataMsg.set_messagecode(st::TunnelMessage_MessageCode_DATA);
            dataMsg.set_ip(client.get_mIp());
            dataMsg.set_submask(client.get_mSubmask());
            dataMsg.set_gateway(client.get_mGateway());
            dataMsg.mutable_packet()->set_packetdata(str.get(), 1024);
            string dnsString = client.get_mDns()[0];
            dataMsg.add_dns(dnsString);
            uint32_t l = dataMsg.ByteSize();
            auto b = new unsigned char[l];
            memset(b, 0, l);
            dataMsg.SerializeToArray(b, l);
            client.writePacket(b, l);
            delete [] b;
//            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
//        if (f.get() != 0) perror("Error Dispatch Event");
    }
    
    cout<<"Error Occured"<<endl;
    
    return 0;
}
