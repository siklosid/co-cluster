#include <stdlib.h>
#include <new>

#include "app.h"

using namespace std;

vector<DestructableAtExit*> __objects_to_destroy_at_exit;
// destroyer function 
void __destroy_at_exit() {
  for(uint32_t i = 0; i < __objects_to_destroy_at_exit.size(); i++) {
    if(__objects_to_destroy_at_exit[i] != NULL) {
      delete __objects_to_destroy_at_exit[i];
    }
  }
  __objects_to_destroy_at_exit.clear();
}

// new handler
void our_new_handler()
{
  log_err( "Not enough memory." );
  abort();
}

// application init
void app_init( const char *prg, const char *prg_group )
{
  set_new_handler( &our_new_handler ); // new_handler is already used in std::
  log_open_syslog( prg, prg_group );
  atexit(&__destroy_at_exit);
}


