#include "Utils/app.h"
#include "Algo/KMeansAlgo.h"

#include "Common/Environment.h"


int main(int argc, char *argv[]) {
  CoClusterParams params;
  app_init(params, argc, argv, PROG_ALL, "co-cluster");


  GlobalInfo *global_info = new GlobalInfo();

  Environment::init(params, global_info);
  Environment::set();

  KMeansAlgo kmeans(params);
  kmeans.Start();

  log_status("Writing out the result:");
  for (uint32_t i = 0; i < global_info->GetNumRowItems(); i++) {
    fprintf(stdout, "%u -> %u", i, global_info->GetRowClust(i));
  }
}
