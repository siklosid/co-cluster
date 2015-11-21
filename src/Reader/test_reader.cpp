#include <vector>

#include "Utils/log.h"
#include "Utils/CoClusterParams.h"
#include "Utils/app.h"
#include "ReaderControl.h"

using std::vector;

CoClusterParams params;

int main(int argc, char* argv[]) {
  app_init(params, argc, argv, PROG_ALL, "co-cluster");

  vector< Data<double>* > *data_set = new vector< Data<double>* >;
  ReaderControl reader_control(params, data_set);
  reader_control.StartRead();
}
