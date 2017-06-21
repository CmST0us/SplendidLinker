//
//  main.cpp
//  server_test
//
//  Created by CmST0us on 17/6/5.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#include <iostream>
#include <map>
#include "TunnelMessage.pb.h"
#include "SLServer.hpp"
#include <event2/bufferevent.h>

int main(int argc, const char * argv[]) {
    //////////
            //Test//
//    std::map<int, std::string> m;
//    m[1] = "asdfs";
//    m.erase(1);
//    return 0;
//    
    //////////
    
    SLServer server;
//    std::map<std::string, std::string> m;
    auto readPacketBlock = [&](SLTunnel::TunnelStatus status, st::TunnelMessage &msg, void *ctx) {
        struct bufferevent *bev = reinterpret_cast<decltype(bev)>(ctx);
        if (msg.messagecode() == st::TunnelMessage_MessageCode_DATA) {
            st::TunnelMessage newMsg;
            newMsg.set_type(st::TunnelMessage_Type_RESPONSE);
            newMsg.set_messagecode(st::TunnelMessage_MessageCode_DATA);
            auto p = newMsg.mutable_packet();
            p->set_packetdata("Hello World", 11);
            int l = newMsg.ByteSize();
            auto b = new unsigned char[l];
            memset(b, 0, l);
            newMsg.SerializeToArray(b, l);
            server.writePacketToIp(msg.ip(), b, l);
            
            delete [] b;
        }
    };
    
    auto estBlock = [&](SLTunnel::TunnelStatus status, st::TunnelMessage &msg, void *ctx) {
        struct bufferevent *bev = reinterpret_cast<decltype(bev)>(ctx);
        if (msg.messagecode() == st::TunnelMessage_MessageCode_ESTABLISH) {
            st::TunnelMessage newMsg;
            newMsg.set_type(st::TunnelMessage_Type_RESPONSE);
            newMsg.set_messagecode(st::TunnelMessage_MessageCode_ESTABLISH);
            newMsg.set_linkencrypttype(st::TunnelMessage_LinkEncryptType_RAW_DATA);
            newMsg.set_ip("10.21.1.2");
            newMsg.set_submask("255.255.255.0");
            newMsg.set_gateway("10.21.1.1");
            newMsg.add_dns("8.8.8.8");
            
            server.get_mClientBufferEvent()["10.21.1.2"] = bev;
            int l = newMsg.ByteSize();
            auto b = new unsigned char[l];
            memset(b, 0, l);
            newMsg.SerializeToArray(b, l);
            bufferevent_write(bev, &l, sizeof(l));
            bufferevent_write(bev, b, l);
            
            delete [] b;
        }
    };
    server.set_mReadPacketCallback(readPacketBlock).set_mEstablishCallback(estBlock);
    server.set_mListenPort(10080);
    
    if (SLTunnel::TunnelStatus::Establish == server.startServer()) {
        std::cout<<"Server Start Up"<<std::endl;
        server.dispatchEvent();
    }
    
    
    
    std::cout<<"Server Error"<<std::endl;
    
    return 0;
}
