//
//  SLUTNInterface.cpp
//  ffffuuunnn
//
//  Created by CmST0us on 17/6/7.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#include "SLUTNInterface.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/sys_domain.h>
#include <net/if_utun.h>
#include <sys/kern_control.h>
#include <sys/ioctl.h>
#include <net/if_var.h>
#include <net/if.h>
#include <unistd.h>
#include <stdlib.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>


//############################

//UTUN 如何判断是否来源目标

//############################
SLUTUNInterface::SLUTUNInterface() {
    this->mUTUNSocket = -1;
    this->mBase = event_base_new();
}

SLUTUNInterface::SLUTUNInterface(const char * addr, std::function<void(std::string err)> & c) {
    int utunSocket = ::socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    if (utunSocket < 0) {
        perror("error create utun socket");
        std::terminate();
    }
    
    //GET UTUN ID
    struct ctl_info kernelControlInfo;
    
    bzero(&kernelControlInfo, sizeof(kernelControlInfo));
    strlcpy(kernelControlInfo.ctl_name, UTUN_CONTROL_NAME, sizeof(kernelControlInfo.ctl_name));
    
    int ret = ::ioctl(utunSocket, CTLIOCGINFO, &kernelControlInfo);
    if (ret < 0) {
        std::string errs = std::string("ioctl failed on kernel control socket: %s\n") + strerror(errno);
        c(errs);
        return;
    }
    
    struct sockaddr_ctl ctrl;
    bzero(&ctrl, sizeof(ctrl));
    
    ctrl.sc_len = sizeof(ctrl);
    ctrl.sc_family = AF_SYSTEM;
    ctrl.ss_sysaddr = AF_SYS_CONTROL;
    ctrl.sc_id = kernelControlInfo.ctl_id;
    ctrl.sc_unit = 0;
    
    ret = connect(utunSocket, (const sockaddr *)&ctrl, sizeof(ctrl));
    if (ret < 0) {
        std::string errs = std::string("error connect utun socket");
        c(errs);
    }
    /// Get the name of a UTUN interface the associated socket.
    char buf[IFNAMSIZ] = {0};
    socklen_t bufSize = IFNAMSIZ;
    ret = getsockopt(utunSocket, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, buf, &bufSize);
    if (ret < 0) {
        std::string errs = std::string("error to set utun opt");
        c(errs);
    }
    
    std::string utunInterfaceName(buf);
    struct in_addr address;
    
    if (inet_pton(AF_INET, addr, &address) == 1) {
        struct ifaliasreq interfaceAliasRequest __attribute__ ((aligned (4)));
        struct in_addr mask = { 0xffffffff };
        int socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
        
        if (socketDescriptor < 0) {
            std::string errs = std::string("Failed to create a DGRAM socket: %s\n") + strerror(errno);
            c(errs);
        }
        
        memset(&interfaceAliasRequest, 0, sizeof(interfaceAliasRequest));
        
        strlcpy(interfaceAliasRequest.ifra_name, buf, sizeof(interfaceAliasRequest.ifra_name));
        
        interfaceAliasRequest.ifra_addr.sa_family = AF_INET;
        interfaceAliasRequest.ifra_addr.sa_len = sizeof(struct sockaddr_in);
        memcpy(&((struct sockaddr_in *)&interfaceAliasRequest.ifra_addr)->sin_addr, &address, sizeof(address));
        
        interfaceAliasRequest.ifra_broadaddr.sa_family = AF_INET;
        interfaceAliasRequest.ifra_broadaddr.sa_len = sizeof(struct sockaddr_in);
        memcpy(&((struct sockaddr_in *)&interfaceAliasRequest.ifra_broadaddr)->sin_addr, &address, sizeof(address));
        
        interfaceAliasRequest.ifra_mask.sa_family = AF_INET;
        interfaceAliasRequest.ifra_mask.sa_len = sizeof(struct sockaddr_in);
        memcpy(&((struct sockaddr_in *)&interfaceAliasRequest.ifra_mask)->sin_addr, &mask, sizeof(mask));
        
        if (ioctl(socketDescriptor, SIOCAIFADDR, &interfaceAliasRequest) < 0) {
            std::string errs = std::string("Failed to set the address of %s interface address to 10.1.21.1: %s\n") + strerror(errno);
            c(errs);
            close(socketDescriptor);
        }
        close(socketDescriptor);
        
    }
    
    mUTUNSocket = utunSocket;
    mUTUNInterfaceName = std::string(buf);
}

static void cb_event(int socket, short event, void *ctx) {
    SLUTUNInterface *utun = reinterpret_cast<SLUTUNInterface *>(ctx);
    if (event & EV_READ) {
        //read event
        std::unique_ptr<unsigned char> buff(new unsigned char[1500]);
        auto readLen = read(utun->get_mUTUNSocket(), buff.get(), 1500);
        utun->get_mReadHandle()(buff.get(), readLen);
    }
}
int SLUTUNInterface::writePacket(const void *data, int len) {
    int writeLen = 0;
    int totalLen = len;
    unsigned char *ucharPtr = (unsigned char *)data;
    while (writeLen < totalLen) {
        writeLen += ::write(this->mUTUNSocket, ucharPtr + writeLen, totalLen - writeLen);
    }
    return writeLen;
}
void SLUTUNInterface::dispatchUTUNPacket(std::function<void (void *, ssize_t)>  readHandle){
    this->mReadHandle = readHandle;
    
#ifdef WIN32
    evthread_use_windows_threads();
#else
    evthread_use_pthreads();
#endif
    
    this->mBase = event_base_new();
    evthread_make_base_notifiable(this->mBase);
    auto event = event_new(this->mBase, this->mUTUNSocket, EV_READ, cb_event, this);
    event_add(event, NULL);
    event_base_dispatch(this->mBase);
}
SLUTUNInterface::~SLUTUNInterface() {
    event_base_loopbreak(this->mBase);
    event_base_free(this->mBase);
    close(this->mUTUNSocket);
}