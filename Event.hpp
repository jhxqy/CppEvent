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


#define DISPATCHER_SELECT



enum class EventBaseType{
    read,write,exception
};

struct EventBase{
    std::function<void(int)> call_back;
    int fd;
    EventBaseType event_type;
public:
    EventBase(int f,EventBaseType et,std::function<void(int)>cb):fd(f),event_type(et),call_back(cb){
        
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
    TimeEvent( std::function<void()> cb,struct timeval &tv);
};


struct TimeEventCompartor{
    bool operator()(TimeEvent *t1,TimeEvent *t2) const;
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

class Dispatcher{
    using TimeEventList=std::priority_queue<TimeEvent*,std::vector<TimeEvent*>,TimeEventCompartor>;

    std::vector<EventBase*> &from_read_list_;
    std::vector<EventBase*> &from_write_list_;
    std::vector<EventBase*> &from_exception_list_;
    std::vector<TimeEvent*> &from_time_events_list_;
    
    std::vector<EventBase*> read_list_;
    std::vector<EventBase*> write_list_;
    std::vector<EventBase*> exception_list_;
    TimeEventList time_events_list_;
    
public:
    Dispatcher(std::vector<EventBase*> &r,std::vector<EventBase*> &w,std::vector<EventBase*> &e,std::vector<TimeEvent*> &t):from_read_list_(r),from_write_list_(w),from_exception_list_(e),from_time_events_list_(t){} 
    
    void dispatch();
    
};
    
#endif

            

class IoContext{
    using TimeEventList=std::priority_queue<TimeEvent*,std::vector<TimeEvent*>,TimeEventCompartor>;
    std::vector<EventBase*> read_list_;
    std::vector<EventBase*> write_list_;
    std::vector<EventBase*> exception_list_;
    std::vector<TimeEvent*> time_events_list_;
    Dispatcher dispatcher;
            

    
public:
    IoContext():dispatcher(read_list_,write_list_,exception_list_,time_events_list_){
        
    }
    void AddEvent(EventBase *e);
    void RemoveEvent(EventBase *e);
    void AddEvent(TimeEvent *e);
    void RemoveEvent(TimeEvent *e);
    
    void Run();
};

;


#endif /* Event_hpp */
