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


class TE{
    Timer &t;
    void f(){
        cout<<"1"<<endl;
        t.AsyncWait(std::bind(&TE::f, this));
    }
public:
    TE(Timer &tt):t(tt){
        t.AsyncWait(std::bind(&TE::f, this));
    }
    
};

int main(int argc, const char * argv[]) {
    IoContext ctx;
    Timer t(ctx);
    t.ExpiresAfter(chrono::seconds(1));
    TE e(t);
    ctx.Run();
   
    return 0;
}
