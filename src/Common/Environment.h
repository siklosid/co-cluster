//! Container for the "environment" that may be needed by most components:

#ifndef __ENVIRONMENT_H
#define __ENVIRONMENT_H


#include <cstdlib>

#include "Utils/CoClusterParams.h"
#include "Common/GlobalInfo.h"


//! Thrown if a singleton-ness related error happens.
class SingletonException {};

//! Class that contains references to the commonly used databases and stuff.
//! As of revision 11501, it is a singleton to make access to it easier.
class Environment {
public:

  ~Environment();
  //! Inits the Environmet object
  static void init(CoClusterParams& params, GlobalInfo *global_info);
  //! Returns params
  inline CoClusterParams &params() { return params_; }
  //! Returns global_info
  inline GlobalInfo* global_info() { return global_info_; }

  //! Returns the singleton Environment instance.
  //! \throw SingletonException if the singleton instance does not exist yet.
  static Environment& get() throw (SingletonException);

  //! Creates and returns the singleton Environment instance with the specified
  //! parameters.
  //! \throw SingletonException if the singleton instance already exists.
  static Environment& set();

  static void CleanUp();


private:
  Environment(CoClusterParams& params, GlobalInfo *global_info)
    : params_(params), global_info_(global_info) {}

  CoClusterParams &params_;
  GlobalInfo *global_info_;

  static Environment* singleton;
};

#endif // __ENVIRONMENT_H
