#include "CommanderFromFile.h"

#include <string.h>
#include <stdio.h>

#include "Utils/log.h"


#define CL_PAUSE      "pause"
#define CL_RESUME       "resume"
#define CL_SET_NUM_THREADS     "threads"

#define CR_BUFF_SIZE       1024

CommanderFromFile::CommanderFromFile(string file_name, AlgoBase &algo)
  : command_(NONE), file_name_(file_name), algo_(algo) {
  remove(file_name_.c_str ());
  log_status("CommanderFromFile initialized.");
}


CommanderFromFile::~CommanderFromFile() {

}


void CommanderFromFile::Main() {
  sleep(30);
  if (command_ == NONE) {
    FILE *F;
    char buf[CR_BUFF_SIZE];

    commander_mutex_.Lock();
    F = fopen(file_name_.c_str(), "rw");
    if(F) {
      if(fgets(buf, CR_BUFF_SIZE - 1, F)) {
        char* eol = strchr(buf, '\n');
        if (eol != NULL) *eol = '\0';

        if(strstr(buf, CL_PAUSE)) {
          log_status("Setting command to PAUSE.");
          command_ = PAUSE;
          algo_.SetNumThreads(0);
          SetCommand();
        } else if(strstr(buf, CL_RESUME)) {
          log_status("Setting command to RESUME.");
          command_ = RESUME;
          algo_.ResumeThreads();
          SetCommand();
        } else if (strstr(buf, CL_SET_NUM_THREADS)) {
          log_status("Setting command to SET_NUM_THREADS.");
          command_ = SET_NUM_THREADS;
          string tmp = strstr(buf, CL_SET_NUM_THREADS);
          if(tmp.length()>=sizeof(CL_SET_NUM_THREADS)) {
            attributes_ = tmp.substr(sizeof(CL_SET_NUM_THREADS));
          } else {
            attributes_ = "";
          }
          algo_.SetNumThreads(atoi(attributes_.c_str()));
          SetCommand();
        } else {
          log_warn("unknown command.");
        }
      }
      fclose(F);
      remove(file_name_.c_str());
    }
    commander_mutex_.Unlock();
  }
}


CommandType CommanderFromFile::GetCommand() {
  CommandType res;

  commander_mutex_.Lock();
  res = command_;
  commander_mutex_.Unlock();

  return res;
}


string CommanderFromFile::GetAttributes() {
  string res;

  commander_mutex_.Lock();
  res = attributes_;
  commander_mutex_.Unlock();

  return res;
}


void CommanderFromFile::SetCommandMutex() {
  commander_mutex_.Lock();
  SetCommand();
  commander_mutex_.Unlock();
}


void CommanderFromFile::SetCommand() {
  command_ = NONE;
}

