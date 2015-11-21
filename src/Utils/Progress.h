#ifndef PROGRESS_H
#define PROGRESS_H

#include <math.h>
#include <stdarg.h>
#include <time.h>
#include "Utils/log.h"


template<class T>
class Progress {
public:
    Progress(T from_, T to_) : from(from_), to(to_), next(0), num_msgs(100) {
        start=time(NULL);
    };
    Progress(T to_) : from(0), to(to_), next(0), num_msgs(100) {
        start=time(NULL);
    };
    Progress(FILE *f) : from(0), next(0), num_msgs(100) {
        fseeko(f, 0, SEEK_END);
        to = ftello(f);
        fseeko(f, 0, SEEK_SET);
        start=time(NULL);
    }
    
    inline void show(T i, const char *fmt, ...) {
        if (next < i) show_inner(i, fmt);
    }
    void show_inner(T i, const char *fmt, ...) {
        char buf[1024];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, 1024, fmt, ap);
        va_end(ap);
        
        time_t now = time(NULL);
        time_t remaining = (time_t)(
            (double)(now - start) / (double)(i - from) * (double)(to - i));
        struct tm *remaining_tm = gmtime(&remaining);
        int secs=remaining_tm->tm_sec;
        int mins=remaining_tm->tm_min;
        int hours=remaining_tm->tm_hour + 24*(remaining_tm->tm_yday);
        
        log_status("%s (%2.2f%% ET: %d:%02d:%02d)", 
            buf, (i-from) * 100.0 / (to - from),
            hours,mins,secs);
        next = (T)( from + 
                (floor((i - from) * num_msgs / (to - from)) + 1.0) * 
                      (to - from) / num_msgs);
    }
    
    T from;
    T to;
    T next;
    float num_msgs;
    time_t start;
};


#endif // PROGRESS_H
