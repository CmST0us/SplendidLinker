//
//  SLClient.cpp
//  SplendidTunnel
//
//  Created by CmST0us on 17/6/4.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/dns.h>
#include <event2/buffer.h>
#include <event2/thread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>

#include "SLClient.hpp"
#include "TunnelMessage.pb.h"

static void cb_bufferevent_event(struct bufferevent *bev, short event, void* ctx) {
    SLClient *tunnel = reinterpret_cast<SLClient *>(ctx);
    switch (event) {
        case BEV_EVENT_EOF: {

        }
            break;
            
        case BEV_EVENT_ERROR:
        case BEV_EVENT_TIMEOUT: {
            if (tunnel != nullptr && tunnel->get_mErrorCallback() != nullptr) tunnel->get_mErrorCallback()(SLTunnel::ServerUnReachable, ctx);
        }
            break;
            
        case BEV_EVENT_READING: {
        
        }
            break;
        case BEV_EVENT_WRITING: {
            
        }
            break;
        case BEV_EVENT_CONNECTED: {
            //write Packetage
            st::TunnelMessage msg;
            msg.set_type(st::TunnelMessage_Type_REQUEST);
            msg.set_messagecode(st::TunnelMessage_MessageCode_ESTABLISH);
            msg.set_username(tunnel->get_mOptions()[OPTIONS_USERNAME_KEY]);
            msg.set_password(tunnel->get_mOptions()[OPTIONS_PASSWORD_KEY]);
            msg.set_passwordencrypttype(st::TunnelMessage_PasswordEncryptType_NO);
            msg.set_linkencrypttype(st::TunnelMessage_LinkEncryptType_RAW_DATA);
            
            uint32_t l = msg.ByteSize();
            unsigned char * b = new unsigned char[l];
            memset(b, 0, l);
            
            msg.SerializeToArray((void *)b, l);
            
//            l = htonl(l);
            bufferevent_write(bev, &l, sizeof(l));
            bufferevent_write(bev, b, l);
            
            delete [] b;
        }
            break;
        default:
            if (tunnel != nullptr && tunnel->get_mErrorCallback() != nullptr) tunnel->get_mErrorCallback()(SLTunnel::ServerUnReachable, tunnel->get_mContext());
            break;
    }
}

static void cb_bufferevent_did_write(struct bufferevent *bev, void *ctx) {
    
}

static void cb_bufferevent_will_read(struct bufferevent *bev, void *ctx) {
    
    auto tunnel = (SLClient *)ctx;
    uint32_t length = 0;
    bufferevent_read(bev, &length, sizeof(length));
//    length = ntohl(length);
    auto payload = new unsigned char [length];
    memset(payload, 0, length);
    bufferevent_read(bev, payload, length);
    
    st::TunnelMessage msg;
    msg.ParseFromArray(payload, length);
    
    if (msg.type() != st::TunnelMessage_Type_RESPONSE) {
        delete [] payload;
        tunnel->get_mErrorCallback()(SLTunnel::EstablishError, &msg);
        bufferevent_free(bev);
        return;
    }
    
    switch (msg.messagecode()) {
        case st::TunnelMessage_MessageCode_ESTABLISH:
            tunnel->set_mIp(msg.ip());
            tunnel->set_mGateway(msg.gateway());
            tunnel->set_mSubmask(msg.submask());
            for (auto d : msg.dns()) {
                tunnel->get_mDns().push_back(d);
            }
            tunnel->get_mEstablishCallback()(SLTunnel::Establish, msg, tunnel->get_mContext());
            break;
        case st::TunnelMessage_MessageCode_AUTHORIZATION:
            tunnel->get_mErrorCallback()(SLTunnel::AuthorizationFaild, tunnel->get_mContext());
            break;
        case st::TunnelMessage_MessageCode_BUSY:
            tunnel->get_mErrorCallback()(SLTunnel::TunnelBusy, tunnel->get_mContext());
            break;
        case st::TunnelMessage_MessageCode_DATA:
            tunnel->get_mReadPacketCallback()(SLTunnel::TunnelData, msg, tunnel->get_mContext());
            break;
        default:
            tunnel->get_mErrorCallback()(SLTunnel::EstablishError, tunnel->get_mContext());
            break;
    }
    
    delete [] payload;
}



SLTunnel::TunnelStatus SLClient::startTunnelWithServerAddress(const char* address, short port) {
    //检查配置
    if (this->mOptions.size() == 0) {
        return SLTunnel::TunnelStatus::ConfigureInvalid;
    }
    
    int socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0 || evutil_make_socket_nonblocking(socket) < 0) {
        return SLTunnel::TunnelStatus::TunnelInitError;
    }
    
    evthread_use_pthreads();
    struct bufferevent *bev = bufferevent_socket_new(mBase, socket, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
    this->mServerBufferEvent = bev;
    bufferevent_setcb(bev, cb_bufferevent_will_read, cb_bufferevent_did_write, cb_bufferevent_event, this);
    
    struct evdns_base *dns = evdns_base_new(mBase, 0);
    if (bufferevent_socket_connect_hostname(bev, dns, AF_INET, address, port) < 0) {
        return SLTunnel::TunnelStatus::TunnelInitError;
    }
    
    bufferevent_enable(bev, EV_READ | EV_WRITE);
    return SLTunnel::TunnelStatus::OnEstablish;
}

void SLClient::writePacket(void *packet, int size) {
    uint32_t l = size;
//    l = htonl(l);
    bufferevent_write(mServerBufferEvent, &l, sizeof(l));
    bufferevent_write(mServerBufferEvent, packet, l);
}
