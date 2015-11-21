#ifndef __MUTEX_H
#define __MUTEX_H

#include <pthread.h>
#include <errno.h>

// Wrapper class for pthread_mutex.
class Mutex{

 public:
  // Initially unlocked.
  Mutex();
  ~Mutex();

  void Lock();
  void Unlock();
  bool IsBusy();
  void TryLockAndContinue ();


 private:
  pthread_mutex_t _mutex;
};


class ConditionMutex {
 public:
  ConditionMutex();
  ~ConditionMutex();

  void Lock();
  void Lock(int max_wait);
  void Unlock();
  void UnlockAll();

  void Enable();
  void Disable();


 private:
  pthread_cond_t cond_;
  pthread_mutex_t mutex_;
  bool condition_;
};


#endif    // __MUTEX_H
