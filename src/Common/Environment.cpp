#include "Common/Environment.h"


Environment* Environment::singleton = NULL;


Environment::~Environment() {
  delete global_info_;
  Environment::CleanUp();
}


void Environment::init(CoClusterParams& params, GlobalInfo *global_info) {
  singleton = new Environment(params, global_info);
}


Environment& Environment::get() throw (SingletonException) {
  if (singleton == NULL) {
    throw SingletonException();
  } else {
    return *singleton;
  }
}

Environment& Environment::set() {
  if (singleton == NULL) {
    throw SingletonException();
  } else {
    return *singleton;
  }
}


void Environment::CleanUp() {
  delete singleton;
}

