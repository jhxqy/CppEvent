//
//  Event.cpp
//  CppEvent
//
//  Created by 贾皓翔 on 2019/11/22.
//  Copyright © 2019 贾皓翔. All rights reserved.
//

#include "Event.hpp"
#include <sys/time.h>
namespace cppnet{
namespace async{
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

bool TimeValCompartor::operator()(const struct timeval &a, const struct timeval &b){
    if(a.tv_sec<b.tv_sec){
        return true;
    }else if(a.tv_sec>b.tv_sec){
        return false;
    }else
        return (a.tv_usec<b.tv_usec);
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
    io_list_.push_back(e);
}

    
        
void IoContext::AddEvent(TimeEvent *e){
    time_events_list_.push_back(e);
}

void IoContext::Run(){
    dispatcher.dispatch();
}


#ifdef DISPATCHER_SELECT
void Dispatcher::dispatch(){
    while (!IOListEmpty()||!TimeListEmpty()) {
        if (!IOListEmpty()) {
            for(auto i:from_io_list_){
                io_list_.push_back(i);
            
            }
            from_io_list_.clear();
        }
        //将新加入的时钟时间加入到time_events_listss中
        TimeEvent *te=nullptr;
        struct timeval tv;
        struct timeval *waitTime=nullptr;
        if(!TimeListEmpty()){
            for(auto i:from_time_events_list_){
                time_events_list_.push(i);
                time_val_list_.insert(i->time);
            }
            from_time_events_list_.clear();
            te=time_events_list_.top();
            tv=*time_val_list_.begin();
            waitTime=&tv;
        }
        
        fd_set read_set;
        fd_set write_set;
        fd_set exception_set;

        __DARWIN_FD_ZERO(&read_set);
        __DARWIN_FD_ZERO(&write_set);
        __DARWIN_FD_ZERO(&exception_set);

        int ioCnt=0;
        for(auto i:io_list_){
            ioCnt=std::max(i->fd, ioCnt);
            switch (i->event_type) {
                case EventBaseType::write:
                    __DARWIN_FD_SET(i->fd,&write_set);
                    break;
                case EventBaseType::read:
                    __DARWIN_FD_SET(i->fd,&read_set);
                    break;
                case EventBaseType::exception:
                    __DARWIN_FD_SET(i->fd,&read_set);
                    break;
                default:
                    break;
            }
        }
        ++ioCnt;
        int select_result=select(ioCnt, &read_set, &write_set, &exception_set, &tv);
        if (select_result>=0) {
            if(te!=nullptr){
                TimeLimit ntl=TimeLimit::NowTimeLimit();
                while(te->timelimit<ntl){
                    te->call_back();
                    time_events_list_.pop();
                    time_val_list_.erase(te->time);
                    delete te;
                    if (!time_events_list_.empty()) {
                        te=time_events_list_.top();
                    }else{
                        break;
                    }
                }
                if (!time_events_list_.empty()) {
                    te=time_events_list_.top();
                    time_events_list_.pop();
                    time_val_list_.erase(te->time);
                    te->time.tv_sec-=tv.tv_sec;
                    te->time.tv_usec-=tv.tv_usec;
                    time_val_list_.insert(te->time);
                    time_events_list_.push(te);
                }
            }
            for(auto i=io_list_.begin();i!=io_list_.end();){
                if ((*i)->event_type==EventBaseType::read&&__DARWIN_FD_ISSET((*i)->fd, &read_set)) {
                    (*i)->call_back((*i)->fd);
                    __DARWIN_FD_CLR((*i)->fd, &read_set);
                    delete (*i);
                    i=io_list_.erase(i);
                }else if((*i)->event_type==EventBaseType::write&&__DARWIN_FD_ISSET((*i)->fd, &write_set)){
                    (*i)->call_back((*i)->fd);
                    __DARWIN_FD_CLR((*i)->fd, &write_set);
                    delete (*i);
                    i=io_list_.erase(i);
                }else if((*i)->event_type==EventBaseType::exception&&__DARWIN_FD_ISSET((*i)->fd, &exception_set)){
                    (*i)->call_back((*i)->fd);
                    __DARWIN_FD_CLR((*i)->fd, &exception_set);
                    delete (*i);
                    i=io_list_.erase(i);
                    
                }else{
                    i++;
                }
            }
        }else if(select_result==-1){
            
        }
        
       
    }
    
}
#endif

}

}
