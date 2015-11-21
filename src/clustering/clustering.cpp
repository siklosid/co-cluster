#include "Utils/log.h"
#include "Utils/CoClusterParams.h"
#include "Utils/app.h"
#include "Utils/CommanderFromFile.h"

#include "Common/Environment.h"
#include "Common/GlobalInfo.h"
////#include "Algo/KMeansAlgo.h"
////#include "Algo/SoftKMeansAlgo.h"
//#include "Algo/ReOrdKMeansAlgo.h"
#include "Algo/AlgoFactory.h"

int main(int argc, char* argv[]) {
  CoClusterParams params;
  app_init(params, argc, argv, CO_CLUSTER, "co-cluster");


  GlobalInfo *global_info = new GlobalInfo();

  Environment::init(params, global_info);
  Environment::set();

  AlgoBase *algo = AlgoFactory::CreateAlgo(params);
  //ReOrdKMeansAlgo *kmeans = new ReOrdKMeansAlgo(params);

  CommanderFromFile commander(string(params.commander_file), *algo);
  commander.Start();

  algo->Start();

  //commander.Stop();
  delete algo;
  // Environment::CleanUp();
}
