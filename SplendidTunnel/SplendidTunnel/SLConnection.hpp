//
//  SLConnection.hpp
//  SplendidTunnel
//
//  Created by CmST0us on 17/6/5.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#ifndef SLConnection_hpp
#define SLConnection_hpp

#include <stdio.h>
#include <iostream>
#include <deque>
#include <thread>
#include <mutex>
#include <event2/bufferevent.h>
#include <event2/event.h>

#include <SplendidTunnel/MarcoTools.h>
#include <SplendidTunnel/TunnelMessage.pb.h>
#include <SplendidTunnel/SLUTNInterface.hpp>
class SLConnection {
    
private:
    std::mutex mDequePopMutex;
    std::mutex mDequePushMutex;
    
    std::deque<st::TunnelMessage> mPacketDeque;

    struct bufferevent *mBufferEvent;
    struct event_base *mBase;

    SLUTUNInterface *mUTUNInterface;
public:
    SLConnection();
    SLConnection(struct bufferevent *be);
    ~SLConnection();
    int resume();
    void writePacket(void *packet, uint32_t l);
    
    void pushPacket(st::TunnelMessage & msg);
    st::TunnelMessage & popPacket();
    GETTER(mUTUNInterface);
    SETTER(mUTUNInterface);
    
    REF_GETTER(mPacketDeque);
};
#endif /* SLConnection_hpp */
