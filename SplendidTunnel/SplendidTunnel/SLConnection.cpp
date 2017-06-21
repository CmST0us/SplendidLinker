//
//  SLConnection.cpp
//  SplendidTunnel
//
//  Created by CmST0us on 17/6/5.
//  Copyright © 2017年 CmST0us. All rights reserved.
//
#include "SLConnection.hpp"
#include <event2/thread.h>

#include <event2/http.h>
#include <event2/buffer.h>

static void cb_event(struct bufferevent *bev, short events, void *ctx)
{
    struct event_base *base = bufferevent_get_base(bev);
    auto tunnel = reinterpret_cast<SLConnection *>(ctx);
//    if (events & BEV_EVENT_EOF) {
////        if (tunnel != nullptr && tunnel->get_mErrorCallback() != nullptr) tunnel->get_mErrorCallback()(SLTunnel::TunnelEnd, ctx);
//        bufferevent_free(bev);
//        event_base_loopbreak(base);
//    } else if (events & BEV_EVENT_ERROR) {
////        if (tunnel != nullptr && tunnel->get_mErrorCallback() != nullptr) tunnel->get_mErrorCallback()(SLTunnel::TunnelError, ctx);
//        
//    }
    bufferevent_free(bev);
//    event_base_loopbreak(base);
//    bufferevent_free(bev);
//    event_base_loopbreak(base);
    /* None of the other events can happen here, since we haven't enabled
     * timeouts */
//    if (tunnel != nullptr && tunnel->get_mErrorCallback() != nullptr) tunnel->get_mErrorCallback()(SLTunnel::TunnelError, ctx);
//    bufferevent_free(bev);
    
}

static void cb_bufferevent_did_write(struct bufferevent *bev, void *ctx) {
    
}

static void cb_bufferevent_will_read(struct bufferevent *bev, void *ctx) {
    auto connection = reinterpret_cast<SLConnection *>(ctx);
    auto ieb = bufferevent_get_input(bev);
    auto oeb = bufferevent_get_output(bev);
    
    uint32_t length = 0;
    auto copyOutLen = evbuffer_copyout(ieb, &length, sizeof(length));
//    length = ntohl(length);
    auto bufLen = evbuffer_get_length(ieb);
    if (copyOutLen < 4) {
        return;
    } else {
        if (length + sizeof(length) > bufLen) {
            return;
        }
    }
    
    bufferevent_read(bev, &length, sizeof(length));
//    length = ntohl(length);
    std::unique_ptr<unsigned char> payload(new unsigned char[length]);
    if (payload.get() == nullptr) {
        //链路损坏
        return;
    }
    bufferevent_read(bev, payload.get(), length);
    st::TunnelMessage msg;
    msg.ParseFromArray(payload.get(), length);
    
    
    if (msg.type() != st::TunnelMessage_Type_REQUEST) {
        return;
    }
    
    switch (msg.messagecode()) {
        case st::TunnelMessage_MessageCode_ESTABLISH: {
            //一定建立成功
            msg.set_type(st::TunnelMessage_Type_RESPONSE);
            msg.set_ip("10.1.21.2");
            msg.set_gateway("10.1.21.2");
            msg.set_submask("255.255.255.0");
            msg.add_dns()->append("8.8.8.8");
            msg.set_messagecode(st::TunnelMessage_MessageCode_ESTABLISH);
            uint32_t msgLen = msg.ByteSize();
            std::unique_ptr<unsigned char> msgBuf(new unsigned char[msgLen]);
            msg.SerializeToArray(msgBuf.get(), msgLen);
            
            bool isUTUNFinish = true;
            std::function<void(std::string)> errcb = [&](std::string err){
                std::cout<<err<<std::endl;
                isUTUNFinish = false;
            };
            SLUTUNInterface *utun = new SLUTUNInterface("10.1.21.2", errcb);
            if (!isUTUNFinish) {
                //建立失败，返回MessageCode;
                perror("建立失败");
                return;
            }
            connection->set_mUTUNInterface(utun);
            auto t = std::thread([&](){
                connection->get_mUTUNInterface()->dispatchUTUNPacket([&](void *data, int size){
                    st::TunnelMessage m;
                    m.set_type(st::TunnelMessage_Type_RESPONSE);
                    m.set_ip("10.1.21.1");
                    m.set_gateway("10.1.21.2");
                    m.set_submask("255.255.255.0");
                    m.add_dns()->append("8.8.8.8");
                    m.set_messagecode(st::TunnelMessage_MessageCode_DATA);
                    m.mutable_packet()->set_packetdata(data, size);
                    
                    uint32_t l = m.google::protobuf::Message::ByteSize();
                    std::unique_ptr<unsigned char> mPtr(new unsigned char[l]);
                    connection->writePacket(mPtr.get(), l);
                });
            });
            t.detach();
            connection->writePacket(msgBuf.get(), msgLen);
        }
//            tunnel->get_mEstablishCallback()(SLTunnel::Establish, msg, bev);
            break;
        case st::TunnelMessage_MessageCode_DATA: {
            
            std::string str = msg.packet().packetdata();
            (connection->get_mUTUNInterface())->writePacket(str.data(), str.size());
//            msg.set_type(st::TunnelMessage_Type_RESPONSE);
//            msg.packet().packetdata();
//            
//            uint32_t msgLen = msg.ByteSize();
//            std::unique_ptr<unsigned char> msgBuf(new unsigned char[msgLen]);
//            msg.SerializeToArray(msgBuf.get(), msgLen);
//            
//            connection->writePacket(msgBuf.get(), msgLen);
        }
//            tunnel->get_mReadPacketCallback()(SLTunnel::TunnelData, msg, bev);
            break;
        default:
//            tunnel->get_mErrorCallback()(SLTunnel::EstablishError, &msg);
            break;
    }
}

SLConnection::SLConnection() {
    this->mUTUNInterface = nullptr;
}

SLConnection::SLConnection(decltype(mBufferEvent) be) {
    this->mUTUNInterface = nullptr;
    this->mBufferEvent = be;
    this->mBase = bufferevent_get_base(be);
    bufferevent_setcb(this->mBufferEvent, cb_bufferevent_will_read, cb_bufferevent_did_write, cb_event, this);
    bufferevent_enable(this->mBufferEvent, EV_READ | EV_WRITE);
    
    std::cout<<"连接初始化完成"<<std::endl;
}
SLConnection::~SLConnection() {
    
    event_base_free(this->mBase);
    std::cout<<"连接释放完成"<<std::endl;
    if (this->mUTUNInterface != nullptr) {
        delete this->mUTUNInterface;
    }
}
int SLConnection::resume() {
    int ret = event_base_dispatch(this->mBase);
    
    //two event
    return ret;
}
void SLConnection::pushPacket(st::TunnelMessage & msg){
    mDequePushMutex.lock();
    this->get_mPacketDeque().push_back(msg);
    mDequePushMutex.unlock();
}
st::TunnelMessage & SLConnection::popPacket() {
    
    mDequePopMutex.lock();
    if (this->mPacketDeque.size() == 0) {
        mDequePopMutex.unlock();
        st::TunnelMessage s;
        return s;
    }
    st::TunnelMessage & m = this->mPacketDeque[0];
    mDequePopMutex.unlock();
    return m;
}
void SLConnection::writePacket(void *packet, uint32_t l){
    uint32_t len = l;
    bufferevent_write(this->mBufferEvent, &len, sizeof(len));
    bufferevent_write(this->mBufferEvent, packet, len);
}