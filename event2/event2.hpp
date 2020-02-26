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
#include <unordered_set>
#include <set>
#include<unistd.h>
#include <cerrno>
#include <iostream>
#define KQUEUE

namespace event{
#define EventTypeMap(XX)                   \
XX(EVENT_WRITE,1)                             \
XX(EVENT_READ,2)                              \
XX(EVENT_PERSIST,4)                           \
XX(EVENT_ET,8)                                \
XX(EVENT_TIMEOUT,16)                          \
XX(EVENT_SIGNAL,32)

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
class EventContext;
struct Event{
    int flag;
    int status;
    evfd_t fd;
    using CallBackType=std::function<void(evfd_t,int)>;
    CallBackType callback;
    struct timeval peroid;
    struct timeval deadline;
    EventContext *context;
    Event(evfd_t f,int fla,const CallBackType&cb):fd(f),flag(fla),callback(cb),status(EventStatus::WAIT){
        
    }
     
};
#ifdef KQUEUE

#include <sys/event.h>
#include <sys/types.h>
struct TimeEventCmpStruct{
    bool operator()(const Event *e1,const Event *e2) const{
        return time::TimeCmp(&(e1->deadline), &(e2->deadline))<0;
    }
};
class Dispatcher{
    std::unordered_set<Event*> new_io_events_list_;
    std::set<Event*,TimeEventCmpStruct> new_time_events_list_;
    int kq=kqueue();

public:
    Dispatcher(){
        
    }
    void AddEvent(Event *e,bool t);
    void RemoveEvent(Event *e){
        
    }
    int Dispatch();
    ~Dispatcher(){
        close(kq);
    }
};

#endif

class EventContext{
    Dispatcher dispatcher_;
    void AddSignal(Event *e){
        if (!(e->flag&EventType::EVENT_SIGNAL)) {
            return;
        }
        signal_event_map_[e->fd].push_back(e);
        signal(e->fd,[](int sig){
//            std::cout<<"进入中断处理函数"<<std::endl;
            write(EventContext::signal_write_fd_, &sig, sizeof(sig));
        });
        signal_n_++;
        return;
        
    }
    
    
    std::unordered_map<int,std::list<Event*>> signal_event_map_;
public:
    static int signal_read_fd_;
    static int signal_write_fd_;
    Event *signal_event_;
    int signal_n_;
    EventContext():signal_n_(0),signal_event_(nullptr){
        int signalfd[2];
        pipe(signalfd);
        signal_read_fd_=signalfd[0];
        signal_write_fd_=signalfd[1];
        signal_event_=new Event(signal_read_fd_,EventType::EVENT_READ,[this](evfd_t fd,int){
        //            std::cout<<"进入x信号事件"<<std::endl;
                    int res;
                    ssize_t size=read(fd, &res, sizeof(res));
                    if (size!=sizeof(res)) {
                        return;
                    }else{
                        std::list<Event*> tempList;
                        auto &ref=this->signal_event_map_[res];
                        tempList.assign(ref.begin(), ref.end());
                        this->signal_event_map_[res].clear();
                        for(auto i:tempList){
                            i->callback(i->fd,i->flag);
                            signal_n_--;

                            if(i->flag&EVENT_PERSIST){
                                i->context->AddEvent(i);
                            }
                            
                        }
                    }
                });
        
    }
    ~EventContext(){
        close(signal_read_fd_);
        signal_read_fd_=-1;
        close(signal_write_fd_);
        signal_write_fd_=-1;
    }

    void AddEvent(Event *e,const timeval &t){
        e->context=this;
        AddSignal(e);
        e->peroid=t;
        time::GetTimeOfDay(&(e->deadline));
        time::TimeAdd(e->deadline, e->peroid, &(e->deadline));
        dispatcher_.AddEvent(e,true);
    }
    void AddEvent(Event *e){
        e->context=this;
        AddSignal(e);
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


inline void Dispatcher::AddEvent(Event *e, bool t){
    if (t) {
        new_time_events_list_.insert(e);
    }
    if(e->fd!=-1){
        if(e->flag&EVENT_SIGNAL){
            if(e->context->signal_n_>1){
                return ;
            }
            e=e->context->signal_event_;
        }
        new_io_events_list_.insert(e);
        struct kevent ee;
        memset(&ee,0, sizeof(ee));
        int ev_filter=0;
        if(e->flag|EventType::EVENT_READ){
            ev_filter|=EVFILT_READ;
        }
        if(e->flag|EventType::EVENT_WRITE){
            ev_filter|=EVFILT_WRITE;
        }
        int flags=0;
        flags|=EV_ENABLE;
        flags|=EV_ADD;
        if(!(e->flag&EVENT_PERSIST)){
            flags|=EV_ONESHOT;
        }
        EV_SET(&ee,e->fd,ev_filter,flags,0,0,e);
        kevent(kq, &ee, 1, nullptr, 0, nullptr);
        if(ee.flags&EV_ERROR){
            throw std::runtime_error(strerror(int(ee.data)));
        }

    }
    
    
}


inline int Dispatcher::Dispatch(){
    struct kevent buf[1024];
    while (new_io_events_list_.size()!=0||new_time_events_list_.size()!=0) {
        int res;
        res=kevent(kq, nullptr, 0, buf, 1024, nullptr);
        if (res<0) {
            if (errno==EINTR) {
                continue;
            }
        }
        if(res>0){
            for (int i=0; i<res; i++) {
                Event* e=(Event*)buf[i].udata;
                if(!(e->flag&EVENT_PERSIST)){
                    new_io_events_list_.erase(e);
                    new_time_events_list_.erase(e);
                }
                e->callback(e->fd,e->flag);
//                用来处理EVENT_PERSIST的成员，再次将其加入；
//                if(e->flag&EVENT_PERSIST){
//                    e->context->AddEvent(e);
//                }
            }
        }
    }
    return 0;
}
};
#endif /* event2_hpp */
