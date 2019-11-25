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

#define DISPATCHER_SELECT

namespace cppnet{
namespace async{
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
            

struct TimeValCompartor{
    bool operator()(const struct timeval &a, const struct timeval &b);
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
    using TimeValList=std::multiset<struct timeval,TimeValCompartor>;
    std::vector<EventBase*> io_list_;
    TimeEventList time_events_list_;

    
    
    std::vector<TimeEvent*> &from_time_events_list_;
    std::vector<EventBase*> &from_io_list_;
    TimeValList time_val_list_;
    
    bool TimeListEmpty(){
        return time_events_list_.empty()&&from_time_events_list_.empty();
    }
    bool IOListEmpty(){
        return io_list_.empty()&&from_io_list_.empty();
    }
public:
    Dispatcher(std::vector<EventBase*> &io,std::vector<TimeEvent*> &t):from_io_list_(io),from_time_events_list_(t){
        
    }
    
    void dispatch();
    
};
    
#endif

            

class IoContext{
    using TimeEventList=std::priority_queue<TimeEvent*,std::vector<TimeEvent*>,TimeEventCompartor>;
    std::vector<EventBase*> io_list_;

    std::vector<TimeEvent*> time_events_list_;
    Dispatcher dispatcher;
            

    
public:
    IoContext():dispatcher(io_list_,time_events_list_){
        
    }
    void AddEvent(EventBase *e);
    void RemoveEvent(EventBase *e);
    void AddEvent(TimeEvent *e);
    void RemoveEvent(TimeEvent *e);
    
    void Run();
};


}
}




#endif /* Event_hpp */
