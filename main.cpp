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

using namespace std;

IoContext ctx;
Timer t(ctx);
Timer t2(ctx);

void print3(){
    cout<<"3秒间隔"<<endl;
    t.AsyncWait(print3);
}

void print1(){
    cout<<"1秒间隔"<<endl;
  //  t2.AsyncWait(print1);
}


int main(int argc, const char * argv[]) {
    
    t.ExpiresAfter(chrono::seconds(3));
    t2.ExpiresAfter(chrono::seconds(1));
    t.AsyncWait(print3);
    t2.AsyncWait(print1);
    ctx.Run();
    return 0;
}
