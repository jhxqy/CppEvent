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
Context ctx;
void doRead();
Event *e=  new Event(fileno(stdin),EVENT_READ,[](evfd_t fd){
    char buf[1024];
    ssize_t n=read(fd, buf, 1024);
    buf[n]=0;
    cout<<buf;
    doRead();
});
void doRead(){
    
    ctx.AddEvent(e,nullptr);
}

int main(){
    Event *e=new Event(fileno(stdin),EVENT_READ|EVENT_PERSIST,[](evfd_t fd){
       
    });
    Event *e2=new Event(-1,EVENT_TIMEOUT|EVENT_PERSIST,[](evfd_t fd){
        cout<<"定时器: 1s"<<endl;
    });
    Event *e3=new Event(-1,EVENT_TIMEOUT|EVENT_PERSIST,[&e2](evfd_t fd){
        cout<<"定时器: 3s"<<endl;
    });
    Event *e4=new Event(-1,EVENT_TIMEOUT,[&e3](evfd_t fd){
          cout<<"定时器: 10s"<<endl;
        ctx.DelEvent(e3);

    });
    timeval t2;
    timeval t3;
    timeval t4;
    time::InitTime(&t2, 1, 0);
    time::InitTime(&t3, 3, 0);
    time::InitTime(&t4, 10,0);
//    ctx.AddEvent(e2, &t2);
//    ctx.AddEvent(e3, &t3);
//    ctx.AddEvent(e4, &t4);
  // ctx.AddEvent(e,nullptr);
    doRead();
    ctx.Run();
   
}
