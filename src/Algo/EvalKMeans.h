/*! Implements model evaluation, where the model is an existing clustering and
    evaluation means that we place the new/test elements into the existing clusters. */


#ifndef _EVAL_KMEANS_H
#define _EVAL_KMEANS_H

#include <map>
#include <fstream>

#include "KMeansAlgo.h"
#include "Utils/Progress.h"
#include "Utils/CoClusterParams.h"
#include "Utils/Mutex.h"
#include "Utils/Timer.h"
#include "Common/DenseData.h"


using std::map;
using std::multimap;

class EvalKMeans : public KMeansAlgo {

 public:
  EvalKMeans(const CoClusterParams &params);
  ~EvalKMeans();

  bool ParallelStep();
  bool NeedMoreIter();
  bool NeedMoreColIter();


 private:
  bool ReadLineFromTest(std::ifstream *input, vector<double> &line);
  bool ReadLineFromTest(std::ifstream *input, map<uint32_t, double> &line);
  double NormalizedSimilarity4(map<uint32_t, double> &data_js,
                               vector<double> &data_l2, uint32_t &min_clust,
                               vector<double> &similarities);

  vector<std::ifstream*> test_data_set_;
  std::ofstream test_clustering_;

  map<uint32_t, long> id2file_pos_;

};


#endif // _EVAL_KMEANS_H
