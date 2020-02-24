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


inline int TimeCmp(const struct timeval *t1,const struct timeval *t2){
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
inline void TimeAdd(const struct timeval &t1,const struct timeval &t2,struct timeval *t3){
    assert(t3!=nullptr);
    t3->tv_sec=t1.tv_sec+t2.tv_sec;
    t3->tv_usec=t1.tv_usec+t2.tv_usec;
    if (t3->tv_usec>1000000) {
        t3->tv_sec+=1;
        t3->tv_usec-=1000000;
    }
}
inline void TimeSub(const struct timeval &t1,const struct timeval &t2,struct timeval *t3){
    assert(t3!=nullptr);
    t3->tv_sec=t1.tv_sec-t2.tv_sec;
    t3->tv_usec=t1.tv_usec-t2.tv_usec;
    if (t3->tv_usec<0) {
        t3->tv_sec-=1;
        t3->tv_usec+=1000000;
    }
}

inline void Timeval2Timespec(const struct timeval &tv,struct timespec *ts){
    ts->tv_sec=tv.tv_sec;
    ts->tv_nsec=tv.tv_usec*1000;
}
inline void Timespec2Timeval(const struct timespec &from,struct timeval *to){
    to->tv_sec=from.tv_sec;
    to->tv_usec=from.tv_nsec/1000;
}


}
}

#endif /* evtime_hpp */
