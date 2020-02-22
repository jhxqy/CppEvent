//
//  event2.hpp
//  CppEvent
//
//  Created by 贾皓翔 on 2020/2/22.
//  Copyright © 2020 贾皓翔. All rights reserved.
//

#ifndef event2_hpp
#define event2_hpp
#include <sys/time.h>
#include <functional>
#include <stdio.h>
#include "type.hpp"
namespace event{
#define EventTypeMap(XX)                   \
XX(EV_WRITE,1)                             \
XX(EV_READ,2)                              \
XX(EV_PERSIST,4)                           \
XX(EV_ET,8)                                \
XX(EV_TIMEOUT,16)                          \
XX(EV_SIGNAL,32)

#define EventStatusMap(XX)                 \
XX(PENDING,1)                              \
XX(ACTIVE,2)                               \
XX(DELETED,4)                              \
XX(WAIT,8)

enum EventType{
#define XX(name,id)                        \
    name=id,
    EventTypeMap(XX)
#undef XX
};

enum EventStatus{
#define XX(name,id)                        \
    name=id,
    EventStatusMap(XX)
#undef XX
};

class Event{
    int flag_;
    int status_;
    evfd_t fd_;
    using CallBackType=std::function<void(evfd_t,int)>;
    CallBackType callback_;
public:
    
    Event(evfd_t fd,int flag,const CallBackType&cb):fd_(fd),flag_(flag),callback_(cb),status_(EventStatus::WAIT){
        
    }
     
};

class EventContext{
    
};

};
#endif /* event2_hpp */
