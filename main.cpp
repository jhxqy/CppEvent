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
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>

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

void S(){
    cout<<"Control C"<<endl;
    ctx.AddSignalEvent(SIGINT,S);
}
int main(int argc, const char * argv[]) {
    t.ExpiresAfter(chrono::seconds(3));
    t2.ExpiresAfter(chrono::seconds(1));
   // t3.ExpiresAfter(chrono::milliseconds(333));
    t.AsyncWait(f1);
    t2.AsyncWait(f2);
 //   t3.AsyncWait(f3);

//    ctx.AddEvent(new EventBase(fileno(stdin),EventBaseType::read,ReadF));
    ctx.AddEvent(new EventBase(fileno(stdin),EventBaseType::read,ReadF));
    ctx.Run();
   
    return 0;
}


