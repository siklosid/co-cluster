#include "ThreadManager.h"
#include "Utils/log.h"


ThreadManager::ThreadManager():
    pause_condition_(false),
    stop_condition_(false),
    running_(false) {}


void ThreadManager::Start() {
  log_dbg("ThreadManager::start called.");

  pthread_attr_t attr;

  // Initialize and set thread detached attribute.
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  const int rc = pthread_create(&thread_, &attr, StaticMain, (void*)this);
  if( rc ) {
    log_err("Return code from pthread_create() is %d.", rc);
    exit(-1);
  }

  // Free attribute
  pthread_attr_destroy(&attr);
  log_dbg("ThreadManager::start ends for thread %x.", thread_);
}


void* ThreadManager::StaticMain(void* arg) {
  ThreadManager* threadManager = (ThreadManager*)arg;

  log_dbg("ThreadManager::static_main called for thread %x.", threadManager->thread_);

  threadManager->running_ = true;
  threadManager->BeforeMain();
  threadManager->FakeMain();
  threadManager->AfterMain();
  threadManager->running_ = false;
  threadManager->stop_condition_ = false;

  log_dbg("ThreadManager::static_main ends for thread %x.", threadManager->thread_);
  pthread_exit(NULL);
  return 0;
}


void ThreadManager::FakeMain() {
  while (!stop_condition_) {
    cond_mutex_.Lock();
    Main();
  }
}



void ThreadManager::WaitForThread(){
  log_dbg("ThreadManager::waitForThread called for thread %x.", thread_);

  const int rc = pthread_join(thread_, NULL);
  if(rc){
    log_err("Return code from pthread_join() is %d.", rc);
    exit(-1);
  }

  log_dbg("ThreadManager::waitForThread ends for thread %x.", thread_);
}



bool ThreadManager::IsRunning() const{
  return running_;
}


void ThreadManager::Pause() {
  cond_mutex_.Enable();
}


void ThreadManager::Resume() {
  cond_mutex_.Disable();
  cond_mutex_.Unlock();
}


void ThreadManager::Stop() {
  stop_condition_ = true;
}


void ThreadManager::Cancel(){
  running_ = false;
  stop_condition_ = false;
  if(pthread_cancel(thread_)){
    log_err("Error in cancelling thread %x.", thread_);
  }else{
    log_dbg("Thread %x cancelled.", thread_);
  }
}
