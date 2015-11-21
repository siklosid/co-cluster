#include <gtest/gtest.h>

#include "Utils/app.h"
#include "Utils/CoClusterParams.h"

//#include "Reader/ReaderTest.h"

//#include "Common/DataTest.h"

//#include "Algo/KMeansAlgoTest.h"

#include "Similarity/L1SimilarityTest.h"
#include "Similarity/L2SimilarityTest.h"
//#include "Similarity/KLSimilarityTest.h"


int main(int argc, char* argv[]) {
  CoClusterParams params;
  ::testing::InitGoogleTest(&argc, argv);
  app_init(params, argc, argv, PROG_ALL, "co-cluster");


  GlobalInfo *global_info = new GlobalInfo();

  Environment::init(params, global_info);
  Environment::set();

  return RUN_ALL_TESTS();
}

