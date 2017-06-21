//
//  SLServer.hpp
//  SplendidTunnel
//
//  Created by CmST0us on 17/6/4.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#ifndef SLServer_hpp
#define SLServer_hpp

#include <stdio.h>

#include <map>
#include <string>
#include <future>
#include <atomic>
#include <event2/bufferevent.h>

#include <SplendidTunnel/MarcoTools.h>
#include <SplendidTunnel/SLTunnel.hpp>
#include <SplendidTunnel/SLConnection.hpp>

class SLServer : public SLTunnel {
private:
    std::map<std::string, std::string> mOptions;
    
    std::map<std::string, struct bufferevent*>mClientBufferEvent;
    
    std::map<int, std::shared_future<int> > mClientConnectionTask;
    
    std::atomic<long> mThreadCount;
    int mListenPort;
    
    void _init();
public:
    SLServer(int listenPort);
    SLServer();
    
    SLTunnel::TunnelStatus startServer();
    SLTunnel::TunnelStatus startServerWithOptions(int listenPort, const decltype(mOptions) &options);
    
    void writePacketToIp(std::string, void *data, int size);
    void releaseConnection(long long taskId);
    
    REF_GETTER(mOptions);
    SETTER(mOptions);
    
    GETTER(mListenPort);
    SETTER(mListenPort);
    
    REF_GETTER(mClientBufferEvent);
    
    REF_GETTER(mClientConnectionTask);
    
    REF_GETTER(mThreadCount);
};

#endif /* SLServer_hpp */
