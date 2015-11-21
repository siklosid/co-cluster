#include <gtest/gtest.h>

#include "Utils/app.h"
#include "Utils/CoClusterParams.h"
#include "Common/Environment.h"

#include "Reader/ReaderTest.h"
#include "Common/DataTest.h"


int main(int argc, char* argv[]) {
  CoClusterParams params;
  app_init(params, argc, argv, PROG_ALL, "co-cluster");


  GlobalInfo *global_info = new GlobalInfo();

  Environment::init(params, global_info);
  Environment::set();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

