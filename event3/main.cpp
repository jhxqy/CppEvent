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
    Event e;
    memset(&e, 0, sizeof(e));
    e.status=INIT;
    e.priority=0;
    e.fd=fileno(stdin);
    e.events=EVENT_READ;
    e.callback=[](evfd_t fd){
        cout<<"callback"<<endl;
        char buf[1204];
        ssize_t n=read(fd, buf, 1204);
        if(n<0){
            cout<<"read error!"<<endl;
            return ;
        }
        buf[n]=0;
        cout<<buf;
    };
    struct timeval t;
    t.tv_sec=5;
    t.tv_usec=0;
    cout<<"时间已加入"<<endl;
    ctx.AddEvent(&e,&t);
    ctx.Run();
    
}
