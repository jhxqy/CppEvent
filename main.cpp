//
//  main.cpp
//  CppEvent
//
//  Created by 贾皓翔 on 2019/11/22.
//  Copyright © 2019 贾皓翔. All rights reserved.
//

#include <iostream>
#include <ctime>
#include <thread>
#include "Event.hpp"
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
using namespace std;

using namespace cppnet::async;

IoContext ctx;
Timer t(ctx);
Timer t2(ctx);
Timer t3(ctx);

void f1(){
    cout<<"3秒间隔 :"<<time(nullptr)<<endl;
    t.AsyncWait(f1);
}

void f2(){
    cout<<"1秒间隔 :"<<time(nullptr)<<endl;
    t2.AsyncWait(f2);
}

void f3(){
    cout<<"0.3秒间隔 :"<<time(nullptr)<<endl;
    t3.AsyncWait(f3);
}
void ReadF(int fd){
    char buf[1024];
    ssize_t n=read(fd, buf, 1024);
    buf[n]=0;
    cout<<"异步读入"<<buf;
}

int main(int argc, const char * argv[]) {
#ifdef  __linux__
    cout<<"linux"<<endl;
#endif
    int epfd=epoll_create(256);
    struct epoll_event e;
    memset(&e,0, sizeof(e));
    e.events=EPOLLIN|EPOLLPRI|EPOLLET;
    e.data.fd=fileno(stdin);

    epoll_ctl(epfd,EPOLL_CTL_ADD,fileno(stdin),&e);
    struct epoll_event events[1024];

    int EN=epoll_wait(epfd,events,1024,-1);
    for(int i=0;i<EN;i++){
        if(events[i].events&EPOLLIN){
            char buf[1024];
            ssize_t len=read(events[i].data.fd,buf,1024);
            buf[len]=0;
            cout<<"异步读入:"<<buf;
            epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
        }
    }
    EN=epoll_wait(epfd,events,1024,-1);
    cout<<EN;
    return 0;
}


