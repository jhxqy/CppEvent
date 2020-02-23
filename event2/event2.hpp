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
#include <list>
#include "type.hpp"
#include "evtime.hpp"
#include <map>
#include <unordered_map>

#include<unistd.h>


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

struct Event{
    int flag;
    int status;
    evfd_t fd;
    using CallBackType=std::function<void(evfd_t,int)>;
    CallBackType callback;
    struct timeval peroid;
    struct timeval deadline;
    Event(evfd_t f,int fla,const CallBackType&cb):fd(f),flag(fla),callback(cb),status(EventStatus::WAIT){
        
    }
     
};

class Dispatcher{
    std::list<Event*> new_io_events_list_;
    std::list<Event*> new_time_events_list_;
public:
    Dispatcher(){
        
    }
    void AddEvent(Event *e,bool t){
        if (t) {
            new_time_events_list_.push_back(e);
        }else{
            new_io_events_list_.push_back(e);
        }
        
    }
    void RemoveEvent(Event *e);
    int Dispatch(){
        return 0;
    }
    ~Dispatcher(){
        
    }
};


class EventContext{
    Dispatcher dispatcher_;
    Event* IfSignal(Event *e){
        if (!(e->flag&EventType::EV_SIGNAL)) {
            return e;
        }
        signal_event_map_[e->fd].push_back(e);
        signal(e->fd,[](int sig){
            write(EventContext::signal_write_fd_, &sig, sizeof(sig));
        });
        Event *newe=new Event(signal_read_fd_,e->flag|EventType::EV_READ|EventType::EV_SIGNAL,[this](evfd_t fd,int){
            int res;
            ssize_t size=read(fd, &res, sizeof(res));
            if (size!=sizeof(res)) {
                return;
            }else{
                for(auto i:this->signal_event_map_[res]){
                    if(i->status&PENDING){
                        i->callback(i->fd,i->flag);
                    }
                }
            }
        });
        return newe;
        
    }
    std::unordered_map<int,std::list<Event*>> signal_event_map_;
public:
    static int signal_read_fd_;
    static int signal_write_fd_;
    EventContext(){
        int signalfd[2];
        pipe(signalfd);
        signal_read_fd_=signalfd[0];
        signal_write_fd_=signalfd[1];
    }
    ~EventContext(){
        close(signal_read_fd_);
        signal_read_fd_=-1;
        close(signal_write_fd_);
        signal_write_fd_=-1;
    }

    void AddEvent(Event *e,const timeval &t){
        e=IfSignal(e);
        e->peroid=t;
        time::GetTimeOfDay(&(e->deadline));
        time::TimeAdd(e->deadline, e->peroid, &(e->deadline));
        dispatcher_.AddEvent(e,true);
    }
    void AddEvent(Event *e){
        e=IfSignal(e);
        dispatcher_.AddEvent(e,false);

    }
    void RemoveEvent(Event *e){
        dispatcher_.RemoveEvent(e);
    }
    void Run(){
        dispatcher_.Dispatch();
    }
    
};

inline int EventContext::signal_read_fd_=-1;
inline int EventContext::signal_write_fd_=-1;

};
#endif /* event2_hpp */
