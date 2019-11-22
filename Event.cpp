//
//  Event.cpp
//  CppEvent
//
//  Created by 贾皓翔 on 2019/11/22.
//  Copyright © 2019 贾皓翔. All rights reserved.
//

#include "Event.hpp"
#include <sys/time.h>

TimeEvent::TimeEvent( std::function<void()> cb,struct timeval &tv):call_back(cb),time(tv){
    struct timeval now;
    
    if(gettimeofday(&now, nullptr)==-1){
        throw std::runtime_error("获取当前时间失败！");
    }
    timelimit.second=now.tv_sec+time.tv_sec;
    timelimit.millisecond=(now.tv_usec)/1000+(time.tv_usec)/1000;
    if (timelimit.millisecond>1000) {
        timelimit.second++;
        timelimit.millisecond%=1000;
    }
    
}




bool TimeEventCompartor::operator()(TimeEvent *t1, TimeEvent *t2) const {
    if(t1->time.tv_sec<t2->time.tv_sec){
        return false;
    }else if(t1->time.tv_sec>t2->time.tv_sec){
        return true;
    }
    
    return (t1->time.tv_usec)>(t2->time.tv_usec);
}

TimeLimit TimeLimit::NowTimeLimit(){
    TimeLimit tl;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    tl.second=tv.tv_sec;
    tl.millisecond=tv.tv_usec/1000;
    return tl;
}



void Timer::ExpiresAfter(const std::chrono::milliseconds &ms){
    t_.tv_sec=ms.count()/1000;
    t_.tv_usec=ms.count()%1000*1000;
}

void Timer::ExpiresAfter(const std::chrono::seconds &s){
    t_.tv_sec=s.count();
    t_.tv_usec=0;
}
void Timer::AsyncWait(std::function<void ()> callback){
    ctx_.AddEvent(new TimeEvent(callback,t_));
    
}

        
void IoContext::AddEvent(EventBase *e){
    switch (e->event_type) {
        case EventBaseType::write:
            write_list_.push_back(e);
            break;
        case EventBaseType::read:
            read_list_.push_back(e);
            break;
        case EventBaseType::exception:
            exception_list_.push_back(e);
            break;
        default:
            break;
    }
}

    
        
void IoContext::AddEvent(TimeEvent *e){
    time_events_list_.push_back(e);
}

void IoContext::Run(){
    dispatcher.dispatch();
}

void Dispatcher::dispatch(){
    while (!from_time_events_list_.empty()||!time_events_list_.empty()) {
        for(auto i:from_time_events_list_){
            time_events_list_.push(i);
        }
        from_time_events_list_.clear();
        
        TimeEvent *te=time_events_list_.top();
        struct timeval t=te->time;
        select(0, nullptr, nullptr, nullptr, &t);
        
        /**
         * 处理时间函数
         */
        TimeLimit ntl=TimeLimit::NowTimeLimit();
        while(te->timelimit<ntl){
            te->call_back();
            time_events_list_.pop();
            delete te;
            if (!time_events_list_.empty()) {
                te=time_events_list_.top();
            }else{
                break;
            }
        }
    }
    
}

