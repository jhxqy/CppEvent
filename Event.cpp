//
//  Event.cpp
//  CppEvent
//
//  Created by 贾皓翔 on 2019/11/22.
//  Copyright © 2019 贾皓翔. All rights reserved.
//

#include "Event.hpp"
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
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
void SignalMap::Signal(int SIG, void (*func)(int)){
    if(!originFunc.count(SIG)){
        originFunc[SIG]=signal(SIG, func);
    }else{
        signal(SIG, func);
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

void IoContext::AddSignalEvent(int SIG, std::function<void ()> cb){
    int pfd[2];
    pipe(pfd);
    SignalMap::Instance()->AddFd(SIG, pfd[1]);
    signal(SIG, [](int SIG){
        char c=0;
        using namespace std;
        for(auto i:SignalMap::Instance()->getFd(SIG)){
            write(i, &c, 1);
        }
        SignalMap::Instance()->rmFd(SIG);
        SignalMap::Instance()->Recover(SIG);
        
    });
    AddEvent(new EventBase(pfd[0],EventBaseType::read,[cb](int){
        cb();
    }));
}

void SignalMap::Recover(int SIG){
    signal(SIG,originFunc[SIG]);
}
void SignalMap::AddFd(int SIG, int fd){
    fds[SIG].push_back(fd);
}
std::list<int> SignalMap::getFd(int SIG){
    return fds[SIG];
}
void SignalMap::rmFd(int SIG){
    fds[SIG].clear();
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

        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_ZERO(&exception_set);

        int ioCnt=0;
        for(auto i:io_list_){
            ioCnt=std::max(i->fd, ioCnt);
            switch (i->event_type) {
                case EventBaseType::write:
                    FD_SET(i->fd,&write_set);
                    break;
                case EventBaseType::read:
                    FD_SET(i->fd,&read_set);
                    break;
                case EventBaseType::exception:
                    FD_SET(i->fd,&read_set);
                    break;
                default:
                    break;
            }
        }
        ++ioCnt;
        int select_result=select(ioCnt, &read_set, &write_set, &exception_set, waitTime);
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
                if ((*i)->event_type==EventBaseType::read&&FD_ISSET((*i)->fd, &read_set)) {
                    (*i)->call_back((*i)->fd);
                    FD_CLR((*i)->fd, &read_set);
                    delete (*i);
                    i=io_list_.erase(i);
                }else if((*i)->event_type==EventBaseType::write&&FD_ISSET((*i)->fd, &write_set)){
                    (*i)->call_back((*i)->fd);
                    FD_CLR((*i)->fd, &write_set);
                    delete (*i);
                    i=io_list_.erase(i);
                }else if((*i)->event_type==EventBaseType::exception&&FD_ISSET((*i)->fd, &exception_set)){
                    (*i)->call_back((*i)->fd);
                    FD_CLR((*i)->fd, &exception_set);
                    delete (*i);
                    i=io_list_.erase(i);
                    
                }else{
                    i++;
                }
            }
        }else if(select_result==-1){
            using namespace std;
        }
        
       
    }
    
}
#endif

}

}
