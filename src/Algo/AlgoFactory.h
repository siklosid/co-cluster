#ifndef _ALGO_FACTORY_H
#define _ALGO_FACTORY_H

#include "KMeansAlgo.h"
#include "SoftKMeansAlgo.h"
#include "ReOrdKMeansAlgo.h"
#include "EvalKMeans.h"


class AlgoFactory {
public:
  static AlgoBase* CreateAlgo(CoClusterParams &params) {
    if (string(params.algo) == "kmeans") {
      log_info("Running with algorithm: kmeans");
      return new KMeansAlgo(params);
    } else if (string(params.algo) == "soft_kmeans") {
      log_info("Running with algorithm: soft_kmeans");
      return new SoftKMeansAlgo(params);
    } else if (string(params.algo) == "reord_kmeans") {
      log_info("Running with algorithm: reord_kmeans");
      return new ReOrdKMeansAlgo(params);
    } else if (string(params.algo) == "eval_kmeans") {
      log_info("Running with algorithm: eval_kmeans");
      return new EvalKMeans(params);
    } else {
      log_err("Undefined algorithm type: %s", params.algo);
      exit(1);
    }
  }
};

#endif // _ALGO_FACTORY_H
