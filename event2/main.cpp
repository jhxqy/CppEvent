//
//  main.cpp
//  CppEvent
//
//  Created by 贾皓翔 on 2020/2/22.
//  Copyright © 2020 贾皓翔. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include "event2.hpp"
#include <thread>

#include<signal.h>
using namespace std;
using namespace event;

EventContext ctx;

void doRead(){
    ctx.AddEvent(new Event(fileno(stdin),EVENT_READ|EVENT_PERSIST,[](evfd_t fd,int flag){
        char buf[1024];
        ssize_t in=read(fd, buf, 1024);
        buf[in]=0;
        cout<<buf;
    }));
}
void Sig(){
    ctx.AddEvent(new Event(SIGINT,EVENT_PERSIST|EVENT_SIGNAL,[](evfd_t fd,int flag){
        cout<<"信号:"<<fd<<"被c触发"<<endl;
    }));
}
int main(){
    doRead();
    Sig();
    cout<<getpid()<<endl;
    ctx.Run();
}
