#ifndef TIMER_H
#define TIMER_H

//#include "time_utils.h" // for our_gettimeofday()
#include <sys/time.h>
#include <stdio.h>

class Timer {
 public:

  // We require explicit begin to start the timer, maybe we should put begin() here
  Timer() {}
    
  inline void begin() {
    clear_pause();
    gettimeofday( &start, NULL );
  } 
   
  inline void pause() {
    gettimeofday( &pausetime, NULL );
  }
   
  inline void cont() { 
    if( !paused() ) return; // no pause;
    timeval cnow;
    gettimeofday( &cnow, NULL );
    start.tv_sec+=cnow.tv_sec-pausetime.tv_sec;
    start.tv_usec+=cnow.tv_usec-pausetime.tv_usec;
    clear_pause();
  }
   
  inline double secs_elapsed() {
    timeval now;
    gettimeofday( &now, NULL );
    if( paused() )
      {
	cont();
	pause();
      }

    return now.tv_sec-start.tv_sec + 
      (now.tv_usec-start.tv_usec)/1000000.0; 
  }

  //
  // Warning: because of ctime, this call is thread unsafe!
  //
  char *estimated_finish_time(double percent) {
    double remaining=secs_elapsed()/percent*(1.0-percent);
    time_t now=time(NULL);
    time_t end=now+(time_t)remaining;
    return ctime(&end);
  }
   
 protected:
   
  inline void clear_pause() {
    pausetime.tv_sec=0;
    pausetime.tv_usec=0;   
  }

  inline bool paused() const {
    return pausetime.tv_sec != 0 || pausetime.tv_usec != 0;
  }
   
  timeval start;
  timeval pausetime;
};


#endif // TIMER_H
