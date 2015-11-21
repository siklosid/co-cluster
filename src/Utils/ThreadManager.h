#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <stdlib.h>
#include <pthread.h>

#include "Mutex.h"

// Wrapper class for pthread.
// Can be used to securely launch and join a thread.



class ThreadManager{

public:
	ThreadManager();
	virtual ~ThreadManager(){};

	// Call this to start the thread.
	void Start();

	// Blocks the caller thread until the started thread terminates.
	// Only the thread that called start can call it after the start!
	void WaitForThread();

	// Returns true if the thread is running.
	bool IsRunning() const;

  // Pause the thread
  void Pause();

  // Resume the thread from pause
  void Resume();

  // Stops the thread syncronously.
  void Stop();

	// Stops the thread asyncronously.
	void Cancel();


protected:
	pthread_t thread_;
	bool pause_condition_;
	bool stop_condition_;
	ConditionMutex cond_mutex_;

	// Overwrite this to implement your functionality.
	// It will be run in a separate thread.
	virtual void BeforeMain() {};
	virtual void Main() = 0;
	virtual void AfterMain() {};
	void FakeMain();


private:
	bool running_;

	// This is called as the new thread.
	static void* StaticMain(void* arg);
};

#endif    // THREADMANAGER_H
