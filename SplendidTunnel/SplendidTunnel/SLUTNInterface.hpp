//
//  SLUTNInterface.hpp
//  ffffuuunnn
//
//  Created by CmST0us on 17/6/7.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#ifndef SLUTNInterface_hpp
#define SLUTNInterface_hpp

#include <iostream>
#include <functional>
#include <string>
#include <stdio.h>
#include <unistd.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
class SLUTUNInterface {
private:
    evutil_socket_t mUTUNSocket;
    std::string mUTUNInterfaceName;
    struct event_base* mBase;
    std::function<void(void *data, ssize_t size)> mReadHandle;
public:
    SLUTUNInterface();
    SLUTUNInterface(const char * addr, std::function<void(std::string err)> & c);
    ~SLUTUNInterface();
    
    int writePacket(const void *data, int len);
    void dispatchUTUNPacket(decltype(mReadHandle) readHandle);
    
    evutil_socket_t get_mUTUNSocket() {
        return mUTUNSocket;
    }
    
    decltype(mReadHandle) & get_mReadHandle() {
        return this->mReadHandle;
    }
    
};
#endif /* SLUTNInterface_hpp */
