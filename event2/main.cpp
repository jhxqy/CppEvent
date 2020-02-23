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
int main(){
    using namespace event;
    EventContext ctx;
    cout<<EventContext::signal_read_fd_<<endl;
}
