#ifndef _COMMANDER_FROM_FILE_H
#define _COMMANDER_FROM_FILE_H

#include <string>

#include "ThreadManager.h"
#include "Mutex.h"

#include "Algo/AlgoBase.h"

using std::string;


typedef enum {
  NONE,
  PAUSE,
  RESUME,
  SET_NUM_THREADS
} CommandType;

class CommanderFromFile : public ThreadManager {

 public:
	CommanderFromFile(string file_name, AlgoBase &algo);
	~CommanderFromFile();

	CommandType GetCommand();
  string GetAttributes();
	void SetCommandMutex();     //sets command to NONE
	void SetCommand();


 private:
  void Main();

	Mutex commander_mutex_;
	CommandType command_;
	const string file_name_;
  string attributes_;
  AlgoBase &algo_;
};

#endif    // _COMMANDER_FROM_FILE_H
