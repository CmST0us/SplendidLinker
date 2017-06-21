/*
 *  SLTunnel.hpp
 *  SLTunnel
 *
 *  Created by CmST0us on 17/6/3.
 *  Copyright © 2017年 CmST0us. All rights reserved.
 *
 */

#ifndef SLTunnel_
#define SLTunnel_

/* The classes below are exported */
#pragma GCC visibility push(default)

#include <string>
#include <event2/event.h>

#include <SplendidTunnel/MarcoTools.h>
#include <SplendidTunnel/TunnelMessage.pb.h>

class SLTunnel {
public:
    enum TunnelStatus {
        Establish,
        ServerUnReachable,
        ConfigureInvalid,

        OnEstablish,
        EstablishError,
        AuthorizationFaild,
        
        TunnelInitError,
        TunnelBusy,
        TunnelEnd,
        TunnelError,
        TunnelData,
    };
protected:
    std::function<void(SLTunnel::TunnelStatus, void *ctx)> mErrorCallback;
    std::function<void(SLTunnel::TunnelStatus, st::TunnelMessage &, void *ctx)> mEstablishCallback;
    std::function<void(SLTunnel::TunnelStatus, st::TunnelMessage &, void *ctx)> mReadPacketCallback;
    std::function<void(SLTunnel::TunnelStatus, st::TunnelMessage &, void *ctx)> mWriteCallback;
    
    std::map<std::string, std::string> mOptions;
    
    event_base *mBase;
    void *mContext;
public:
    SLTunnel();
    
    int dispatchEvent();
    
    GETTER(mBase);
    SETTER(mBase);
    GETTER(mContext);
    SETTER(mContext);
    
    BUILDER_SETTER(SLTunnel, mErrorCallback);
    GETTER(mErrorCallback);
    
    BUILDER_SETTER(SLTunnel, mEstablishCallback);
    GETTER(mEstablishCallback);
    
    BUILDER_SETTER(SLTunnel, mReadPacketCallback);
    GETTER(mReadPacketCallback);
    
    BUILDER_SETTER(SLTunnel, mWriteCallback);
    GETTER(mWriteCallback);
    
    REF_GETTER(mOptions);
    SETTER(mOptions);
};

#pragma GCC visibility pop
#endif
