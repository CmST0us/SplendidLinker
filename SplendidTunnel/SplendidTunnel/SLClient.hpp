//
//  SLClient.hpp
//  SplendidTunnel
//
//  Created by CmST0us on 17/6/4.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#ifndef SLClient_hpp
#define SLClient_hpp

#include <functional>
#include <map>
#include <event2/bufferevent.h>
#include <string>
#include <vector>

#include <SplendidTunnel/SLTunnel.hpp>
#include <SplendidTunnel/MarcoTools.h>
#include <SplendidTunnel/TunnelMessage.pb.h>

#define OPTIONS_USERNAME_KEY "username"
#define OPTIONS_PASSWORD_KEY "password"
#define OPTIONS_LINK_ENCRYPT_TYPE_KEY "link-encrypt-type"
#define OPTIONS_PASSWORD_ENCRYPT_TYPE_KEY "link-encrypt-type"


class SLClient: public SLTunnel {
private:
    struct bufferevent* mServerBufferEvent;
    std::string mIp;
    std::string mGateway;
    std::string mSubmask;
    std::vector<std::string> mDns;
    
public:
    SLClient() {
        
    }
    SLClient(void *context) {
        this->mContext = context;
    }
    
    SLClient(void *contest, const decltype(mOptions) &options) {
        this->mContext = contest;
        this->mOptions = options;
    }
    
    static SLClient ClientBuilder() {
        SLClient c;
        return c;
    }
    
    SLClient & addOptions(decltype(mOptions.begin()->first) key, decltype(mOptions.begin()->second) value) {
        mOptions[key] = value;
        return *this;
    }
    
    void writePacket(void *packet, int size);
    
    TunnelStatus startTunnelWithServerAddress(const char *address, short port);
    
    SETTER(mIp);
    REF_GETTER(mIp);
    
    SETTER(mGateway);
    REF_GETTER(mGateway);
    
    SETTER(mSubmask);
    GETTER(mSubmask);
    
    REF_GETTER(mDns);
    
    SETTER(mServerBufferEvent);
    GETTER(mServerBufferEvent);
};

#endif /* SLClient_hpp */
