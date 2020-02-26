//
//  event.hpp
//  CppEvent
//
//  Created by 贾皓翔 on 2020/2/26.
//  Copyright © 2020 贾皓翔. All rights reserved.
//

#ifndef event_hpp
#define event_hpp

#include <stdio.h>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <string>
#include <set>
#include "evtime.hpp"
#include "type.hpp"
#include <list>
#include <cerrno>
#include <unistd.h>
#include <queue>
#include <vector>
#ifdef __APPLE__
#include <sys/event.h>
#include <sys/types.h>
#endif

namespace event{

#define EventTypeMap(XX)                   \
XX(EVENT_TIMEOUT,0x01)                           \
XX(EVENT_WRITE,0x02)                             \
XX(EVENT_READ,0x04)                              \
XX(EVENT_PERSIST,0x08)                           \
XX(EVENT_ET,0x10)                                \
XX(EVENT_SIGNAL,0x20)

#define EventStatusMap(XX)                 \
XX(PENDING,0x01)                              \
XX(ACTIVE,0x02)                               \
XX(INIT,0x04)
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
    using EventCallBackType=std::function<void(evfd_t)>;
    
    int status;
    int events;
    int priority;
    evfd_t fd;
    struct timeval peroid;
    struct timeval deadline;
    EventCallBackType callback;
    
    
};

struct time_event_cmp{
    bool operator()(const Event *e1,const Event *e2)const;
};

inline bool time_event_cmp::operator()(const Event *e1, const Event *e2)const{
    int result=time::TimeCmp(&(e1->deadline), &(e2->deadline));
    if(result==0){
        return (e1<e2);
    }else{
        return (result<0);
    }
}

class EpollBodyInterface;
class Context{
    EpollBodyInterface *ebi;
public:
    Context();
//    超时事件等待列表列表
    std::multiset<Event*,time_event_cmp> time_event_list_;
//    IO时间等待列表
    std::unordered_set<Event*> io_event_list_;
//    已激活事件列表
    std::list<Event*> active_event_list_;
    ~Context();

    int AddEvent(Event *e,struct timeval *tv);
    int DelEvent(Event *e);
    int Run();
};

class SignalManager{
public:
    evfd_t sockpair[2];
    SignalManager(){
        if (pipe(sockpair)){
            throw std::runtime_error("signal manager创建管道失败:"+std::string(strerror(errno)));
        }
        /**
         sockpair[0] 用于读
         sockpair[1] 用于写
         */
    }
    Event *GetSignalEvent(){
        static Event e;
        return &e;
    }
    ~SignalManager(){
        close(sockpair[0]);
        close(sockpair[1]);
    }
};


class EpollBodyInterface{
public:

    virtual int AddEvent(Event *p)=0;
    virtual int DelEvent(Event*)=0;
    virtual int dispatch(struct timeval *tv)=0;
    virtual ~EpollBodyInterface(){
    }
};


#ifdef __APPLE__
class KqueueImple:public EpollBodyInterface{
    Context *ctx_;
    int kq;
    const int BUF_MAXN=1024;
public:
    KqueueImple(Context *ctx):ctx_(ctx){ 
        kq=kqueue();
        if(kq==-1){
            throw std::runtime_error(strerror(errno));
        }
    }
    int AddEvent(Event *e) override{
        struct kevent ee;
        memset(&ee, 0, sizeof(ee));
        int ev_filter=0;
        if(e->events|EventType::EVENT_READ){
            ev_filter|=EVFILT_READ;
        }
        if(e->events|EventType::EVENT_WRITE){
            ev_filter|=EVFILT_WRITE;
        }
        int flags=0;
        flags|=EV_ENABLE;
        flags|=EV_ADD;
        if(!(e->events&EVENT_PERSIST)){
            flags|=EV_ONESHOT;
        }
        EV_SET(&ee,e->fd,ev_filter,flags,0,0,e);
        int res=kevent(kq, &ee, 1, nullptr, 0, nullptr);
        if(res==-1||ee.flags&EV_ERROR){
            return -1;
        }
        return 0;
    }
    int DelEvent(Event  *e)override{
        return 0;
    }
    int dispatch(struct timeval *val) override{
        struct kevent buf[BUF_MAXN];
        struct timespec tspec;
        time::Timeval2Timespec(*val, &tspec);
        int res=kevent(kq,nullptr,0,buf,BUF_MAXN,&tspec);
        if(res<0){
            return -1;
        }
        if(res==0){
            return 0;
        }
        for(int i=0;i<res;i++){
            Event *e=(Event*)( buf[i].udata);
            ctx_->active_event_list_.push_back(e);
        }
        return res;
        
    }
    ~KqueueImple(){
        close(kq);
    }
    
    
};

#endif


/* object function imple*/


inline Context::Context(){
    ebi=new KqueueImple(this);
    
}
inline Context::~Context(){
    if(ebi!=nullptr){
        delete ebi;
    }
}

inline int Context::AddEvent(Event *e, struct timeval *tv){
    if(tv!=nullptr){
        e->events|=EVENT_TIMEOUT;
        e->peroid=*tv;
        time::GetTimeOfDay(&(e->deadline));
        time::TimeAdd(e->peroid, e->deadline, &(e->deadline));
        time_event_list_.insert(e);
    }
    if(!(e->events&(EVENT_READ|EVENT_WRITE))){
        io_event_list_.insert(e);
        return ebi->AddEvent(e);
    }
    
    
    /**
    TODO:
     信号部分
     */
    
    return 0;
}

inline int Context::DelEvent(Event *e){
    //先判断是否在timeevent和ioevent当中，若存在，则从中删除，并从epoll中删除
    bool existed=false;;
    if(time_event_list_.count(e)){
        time_event_list_.erase(e);
        existed=true;
    }
    if(io_event_list_.count(e)){
        time_event_list_.erase(e);
        existed=true;
    }
    /*
     若存在并删除返回0，否则返回-1；
     */
    if(existed){
        
        return ebi->DelEvent(e);
    }else{
        return -1;
    }
    
    
}

inline int Context::Run(){
    
}


}



#endif /* event_hpp */
