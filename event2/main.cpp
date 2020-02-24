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
using namespace std;
using namespace event;

EventContext ctx;

void doRead(){
    ctx.AddEvent(new Event(fileno(stdin),EVENT_READ,[](evfd_t fd,int flag){
        char buf[1024];
        ssize_t in=read(fd, buf, 1024);
        buf[in]=0;
        cout<<buf;
        doRead();
    }));
}
int main(){
    doRead();
    ctx.Run();
}
