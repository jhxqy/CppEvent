//
//  Event.hpp
//  CppEvent
//
//  Created by 贾皓翔 on 2019/11/22.
//  Copyright © 2019 贾皓翔. All rights reserved.
//

#ifndef Event_hpp
#define Event_hpp

#include <stdio.h>
#include <functional>
#include <chrono>
#include <vector>
#include <list>
#include <queue>
#include <set>
#include <unordered_map>

#ifdef __APPLE__
#define DISPATCHER_KQUEUE
#endif


#ifdef  __linux__
#define DISPATCHER_EPOLL
#endif



namespace cppnet{
namespace async{
enum class EventBaseType{
    read,write,exception,signal
};

struct EventBase{
    std::function<void(int)> call_back;
    int fd;
    EventBaseType event_type;
public:
    EventBase(int f,EventBaseType et,const std::function<void(int)>&cb):fd(f),event_type(et),call_back(cb){
        
    }
};


struct TimeLimit{
    long second;  //秒
    long millisecond; //毫秒
    static TimeLimit NowTimeLimit();
    
    bool operator <(const TimeLimit &t)const {
        if(second<t.second){
            return true;
        }else if(second>t.second){
            return false;
        }else{
            return (millisecond<t.millisecond);
        }
    }
    
};
            
struct TimeEvent{
    std::function<void()> call_back;
    struct timeval time;
    TimeLimit timelimit;
public:
    TimeEvent( const std::function<void()> &cb,struct timeval &tv);
};


struct TimeEventCompartor{
    bool operator()(TimeEvent *t1,TimeEvent *t2) const;
};
            

struct TimeValCompartor{
    bool operator()(const struct timeval &a, const struct timeval &b)const ;
};

            
class IoContext;
class Timer{
    struct timeval t_;
    IoContext &ctx_;
public:
    Timer(IoContext &ctx):ctx_(ctx){
    }
    //毫秒计时
    void ExpiresAfter(const std::chrono::milliseconds &ms);
    //秒计时
    void ExpiresAfter(const std::chrono::seconds&s);
    
    //启动事件
    void AsyncWait(std::function<void()>);
    

};
#ifdef DISPATCHER_SELECT
#include <sys/select.h>

class Dispatcher{
    using TimeEventList=std::priority_queue<TimeEvent*,std::vector<TimeEvent*>,TimeEventCompartor>;
    using TimeValList=std::multiset<struct timeval,TimeValCompartor>;
    std::list<EventBase*> io_list_;
    TimeEventList time_events_list_;

    
    
    std::list<TimeEvent*> &from_time_events_list_;
    std::list<EventBase*> &from_io_list_;
    TimeValList time_val_list_;
    
    bool TimeListEmpty(){
        return time_events_list_.empty()&&from_time_events_list_.empty();
    }
    bool IOListEmpty(){
        return io_list_.empty()&&from_io_list_.empty();
    }
public:
    Dispatcher(std::list<EventBase*> &io,std::list<TimeEvent*> &t):from_io_list_(io),from_time_events_list_(t){
        
    }
    
    void Dispatch();
    
};
    
#endif


#ifdef DISPATCHER_EPOLL
#include <sys/epoll.h> 
class Dispatcher{
    using TimeEventList=std::priority_queue<TimeEvent*,std::vector<TimeEvent*>,TimeEventCompartor>;
    using TimeValList=std::multiset<struct timeval,TimeValCompartor>;
    std::list<EventBase*> &io_list_;
    TimeEventList time_events_list_;
    std::list<TimeEvent*> &from_time_events_list_;
    TimeValList time_val_list_;
    const static int MAXN=1024;
    int epfd;
    int event_number;


    bool IOEventEmpty(){
        return io_list_.empty()&&event_number==0;
    }
    bool TimeEventEmpty(){
        return from_time_events_list_.empty()&&time_events_list_.empty();
    }
public:
    Dispatcher(std::list<EventBase*>&io,std::list<TimeEvent*> &t):io_list_(io),from_time_events_list_(t){
        epfd=epoll_create(MAXN);
        event_number=0;
    }
    void Dispatch();
    ~Dispatcher();
};
#endif


#ifdef DISPATCHER_KQUEUE

#include <sys/event.h>
#include <sys/types.h>
typedef long time_t;
#ifndef _TIMESPEC
#define _TIMESPEC
struct timespec {
time_t tv_sec; // seconds
long tv_nsec; // and nanoseconds
};
#endif
class Dispatcher{

    using TimeEventList=std::priority_queue<TimeEvent*,std::vector<TimeEvent*>,TimeEventCompartor>;
    using TimeValList=std::multiset<struct timeval,TimeValCompartor>;
    std::list<EventBase*> &io_list_;
    TimeEventList time_events_list_;
    std::list<TimeEvent*> &from_time_events_list_;
    TimeValList time_val_list_;
    const static int MAXN=1024;
    int kid;
    int event_number;

    bool IOEventEmpty(){
        return io_list_.empty()&&event_number==0;
    }
    bool TimeEventEmpty(){
        return from_time_events_list_.empty()&&time_events_list_.empty();
    }
public:
    Dispatcher(std::list<EventBase*>&io,std::list<TimeEvent*>&t):io_list_(io),from_time_events_list_(t){
        kid=kqueue();
        if(kid==-1){
            throw std::runtime_error("kqueue fd create error!");
        }
        event_number=0;
    }
    void Dispatch();
    ~Dispatcher();

};
#endif









class IoContext{
    using TimeEventList=std::priority_queue<TimeEvent*,std::vector<TimeEvent*>,TimeEventCompartor>;
    std::list<EventBase*> io_list_;

    std::list<TimeEvent*> time_events_list_;
    Dispatcher dispatcher;
            

    
public:
    IoContext():dispatcher(io_list_,time_events_list_){
        
    }
    void AddEvent(EventBase *e);
    void RemoveEvent(EventBase *e);
    void AddEvent(TimeEvent *e);
    void RemoveEvent(TimeEvent *e);
    void AddSignalEvent(int SIG,std::function<void(void)>cb);
    void Run();
};

class SignalMap{
    std::unordered_map<int,std::list<int>> fds;
    SignalMap(){};
    std::unordered_map<int,void(*)(int)> originFunc;
public:
    static SignalMap* Instance(){
        static SignalMap m;
        return &m;
    }
    
    void Signal(int SIG,void(*func)(int));
    void Recover(int SIG);
    void AddFd(int SIG,int fd);
    std::list<int> getFd(int SIG);
    void rmFd(int SIG);
    
};


}
}




#endif /* Event_hpp */
