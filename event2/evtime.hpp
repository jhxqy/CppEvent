//
//  evtime.hpp
//  CppEvent
//
//  Created by 贾皓翔 on 2020/2/22.
//  Copyright © 2020 贾皓翔. All rights reserved.
//

#ifndef evtime_hpp
#define evtime_hpp
#include <sys/time.h>
#include <stdio.h>
#include <cassert>
namespace event{
namespace time{

inline int GetTimeOfDay(struct timeval *t){
    return ::gettimeofday(t, nullptr);
}


inline int TimeCmp(struct timeval *t1,struct timeval *t2){
    if (t1->tv_sec<t2->tv_sec) {
        return -1;
    }else if(t1->tv_sec>t2->tv_sec){
        return 1;
    }else if(t1->tv_usec<t2->tv_usec){
        return -1;
    }else if(t1->tv_usec>t2->tv_usec){
        return 1;
    }else{
        return 0;
    }
}
inline void TimeAdd(struct timeval &t1,struct timeval &t2,struct timeval *t3){
    assert(t3!=nullptr);
    t3->tv_sec=t1.tv_sec+t2.tv_sec;
    t3->tv_usec=t1.tv_usec+t2.tv_usec;
    if (t3->tv_usec>1000000) {
        t3->tv_sec+=1;
        t3->tv_usec-=1000000;
    }
}
inline void TimeSub(struct timeval &t1,struct timeval &t2,struct timeval *t3){
    assert(t3!=nullptr);
    t3->tv_sec=t1.tv_sec-t2.tv_sec;
    t3->tv_usec=t1.tv_usec-t2.tv_usec;
    if (t3->tv_usec<0) {
        t3->tv_sec-=1;
        t3->tv_usec+=1000000;
    }
}




}
}

#endif /* evtime_hpp */
