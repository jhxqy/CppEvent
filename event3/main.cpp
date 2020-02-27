//
//  main.cpp
//  CppEvent
//
//  Created by 贾皓翔 on 2020/2/26.
//  Copyright © 2020 贾皓翔. All rights reserved.
//

#include <stdio.h>

#include <iostream>
#include "evtime.hpp"
#include "event.hpp"
using namespace std;
using namespace event;
int main(){
    Context ctx;
    Event *e=new Event(fileno(stdin),EVENT_READ|EVENT_PERSIST,[](evfd_t fd){
        char buf[1024];
        ssize_t n=read(fd, buf, 1024);
        buf[n]=0;
        cout<<buf;
    });
    Event *e1=new Event(-1,EVENT_TIMEOUT|EVENT_PERSIST,[](evfd_t fd){
        cout<<"定时器: 1s"<<endl;
    });
    
    struct timeval t;
    struct timeval t2;
    t2.tv_sec=1;
    t2.tv_usec=0;
    t.tv_sec=5;
    t.tv_usec=0;
    cout<<"时间已加入"<<endl;
    ctx.AddEvent(e1,&t2 );
    ctx.AddEvent(e,nullptr);
    ctx.Run();
    
}
