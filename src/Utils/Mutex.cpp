#include <sys/time.h>

#include "Mutex.h"

Mutex::Mutex(){
	pthread_mutex_init(&_mutex, NULL);
}


Mutex::~Mutex(){
	pthread_mutex_destroy(&_mutex);
}


void Mutex::Lock(){
	pthread_mutex_lock(&_mutex);
}


void Mutex::Unlock(){
	pthread_mutex_unlock(&_mutex);
}


bool Mutex::IsBusy() {
    return(pthread_mutex_trylock(&_mutex) == EBUSY);
}


void Mutex::TryLockAndContinue() {
    // if we do care for success, use isBusy () above
    (void) pthread_mutex_trylock(&_mutex);
}


// Conditional mutex implementation
ConditionMutex::ConditionMutex() :
    condition_(false) {
  pthread_mutex_init(&mutex_, NULL);
  pthread_cond_init(&cond_, NULL);
}


ConditionMutex::~ConditionMutex() {
  pthread_mutex_destroy(&mutex_);
  pthread_cond_destroy(&cond_);
}


void ConditionMutex::Lock() {
  pthread_mutex_lock(&mutex_);
  if (condition_) {
    pthread_cond_wait(&cond_, &mutex_);
  }
  pthread_mutex_unlock(&mutex_);
}


void ConditionMutex::Lock(int max_wait) {

  struct timeval *tp = NULL;
  struct timespec tv;

  gettimeofday(tp, NULL);
  /* Construct the timespec from the number of whole seconds... */
  tv.tv_sec = (size_t)max_wait;
  //tv.tv_nsec = (long)((max_wait - tv.tv_sec)*1e+9);
  tv.tv_sec += tp->tv_sec;
  tv.tv_nsec = tp->tv_usec*1000 + 1000000;

  delete tp;
//   tv.tv_sec = tp.tv_sec + max_wait;
//   tv.tv_nsec = tp.tv_usec*1000;

  pthread_mutex_lock(&mutex_);
  if (condition_) {
    pthread_cond_timedwait(&cond_, &mutex_, &tv);
  }
  pthread_mutex_unlock(&mutex_);
}


void ConditionMutex::Unlock() {
  pthread_mutex_lock(&mutex_);
  pthread_cond_signal(&cond_);
  pthread_mutex_unlock(&mutex_);
}


void ConditionMutex::UnlockAll() {
  pthread_mutex_lock(&mutex_);
  pthread_cond_broadcast(&cond_);
  pthread_mutex_unlock(&mutex_);
}


void ConditionMutex::Enable() {
  pthread_mutex_lock(&mutex_);
  condition_ = true;
  pthread_mutex_unlock(&mutex_);
}


void ConditionMutex::Disable() {
  pthread_mutex_lock(&mutex_);
  condition_ = false;
  pthread_mutex_unlock(&mutex_);
}
