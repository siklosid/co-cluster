#ifndef APPINIT_H
#define APPINIT_H

#include <vector>

#include <inttypes.h>

#include "Utils/log.h"
using std::vector;

// Sets new handler, opens syslog channel with proper identity
void app_init( const char *prg = LOG_DEF_PRG,
               const char *prg_group = LOG_DEF_PRG_GROUP );

template <class ParamsT>
void app_init( ParamsT &params, int argc, char *argv[],
               const uint32_t prog_mask,
               const char *prg = LOG_DEF_PRG,
               const char *prg_group = LOG_DEF_PRG_GROUP );

//! Abstract class for object that must be destroyed at exit
//! (normal exit, ctrl+c, shutdown)
class DestructableAtExit {
 public:
  virtual ~DestructableAtExit() {};
};
// list of objects to destroy
extern vector<DestructableAtExit*> __objects_to_destroy_at_exit;
inline void destroy_at_exit(DestructableAtExit* obj) {
  __objects_to_destroy_at_exit.push_back(obj);
}

inline void do_not_destroy_at_exit(DestructableAtExit* obj) {
  for(size_t i = 0; i < __objects_to_destroy_at_exit.size(); ++i) {
    if (__objects_to_destroy_at_exit[i] == obj) {
      __objects_to_destroy_at_exit[i] = NULL;
    }
  }
}


// Template implementation
#include "app.tcc"

#endif // !APPINIT_H

