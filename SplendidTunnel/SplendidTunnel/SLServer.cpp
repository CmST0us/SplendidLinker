//
//  SLServer.cpp
//  SplendidTunnel
//
//  Created by CmST0us on 17/6/4.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#include "SLServer.hpp"
#include "SLConnection.hpp"
#include "TunnelMessage.pb.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <future>
#include <utility>


static void cb_connlisterner(struct evconnlistener *listener, evutil_socket_t s, struct sockaddr * addr, int socklen, void * ctx) {
    SLServer *server = reinterpret_cast<SLServer *>(ctx);
    
    auto &count = server->get_mThreadCount();
    if (count > 1000) {
        //并发切换等待
        return;
    }
    struct event_base *clientEventBase = event_base_new();
    struct bufferevent *bev = bufferevent_socket_new(clientEventBase, s, BEV_OPT_CLOSE_ON_FREE); //记得这个CLOSE ON FREE
    
    if (!bev) {
        fprintf(stderr, "Error constructing bufferevent!");
#warning TODO: ERROR CALLBACK
        return;
    }
    
    ++count;
    auto t = std::thread([=, &count](){
        auto clientConnection = new SLConnection(bev);
        int ret = clientConnection->resume();
        delete clientConnection;
        --count;
        return ret;
    });
    t.detach();
    
}

void SLServer::_init() {
    
}

SLServer::SLServer() {
    this->_init();
}

SLServer::SLServer(int listenPort) {
    this->_init();
    this->mListenPort = listenPort;
}

SLTunnel::TunnelStatus SLServer::startServer() {
    return this->startServerWithOptions(this->mListenPort, this->mOptions);
}

SLTunnel::TunnelStatus SLServer::startServerWithOptions(int listenPort, const decltype(mOptions) &options) {
    this->mOptions = options;
    this->mListenPort = listenPort;
    
    auto socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        return SLTunnel::TunnelStatus::ConfigureInvalid;
    }
    std::string server = "";
    if (this->mOptions.count("server") == 0) {
        server = "0.0.0.0";
    } else {
        server = this->mOptions["server"];
    }
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(listenPort);
    addr.sin_addr.s_addr = inet_addr(server.c_str());
    auto listener = evconnlistener_new_bind(this->get_mBase(), cb_connlisterner, this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (const sockaddr *)&addr, sizeof(addr));
    if (listener == nullptr) {
        return SLTunnel::TunnelStatus::ConfigureInvalid;
    }
    
    return SLTunnel::TunnelStatus::Establish;
}

void SLServer::writePacketToIp(std::string s, void *data, int size) {
    struct bufferevent *bev = this->mClientBufferEvent[s];
    uint32_t l = size;
//    l = htonl(l);
    bufferevent_write(bev, &l, sizeof(l));
    bufferevent_write(bev, data, l);
}

void SLServer::releaseConnection(long long taskId) {
        //必须在线程被释放之后才能通知，否则futured的析构会卡死earse
}