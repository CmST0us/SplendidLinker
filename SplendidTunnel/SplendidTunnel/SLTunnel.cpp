/*
 *  SplendidTunnel.cpp
 *  SplendidTunnel
 *
 *  Created by CmST0us on 17/6/3.
 *  Copyright © 2017年 CmST0us. All rights reserved.
 *
 */

#include <iostream>
#include <event2/event.h>
#include <event2/thread.h>

#include "SLTunnel.hpp"

using namespace std;

SLTunnel::SLTunnel() {
#ifdef __WIN32
    evthread_use_windows_threads();
#else
    evthread_use_pthreads();
#endif
    this->mBase = event_base_new();
    evthread_make_base_notifiable(this->mBase);
}

int SLTunnel::dispatchEvent() {
    return event_base_dispatch(this->mBase);
}