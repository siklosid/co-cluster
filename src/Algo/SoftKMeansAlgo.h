#ifndef _SOFT_KMEANS_ALGO_H
#define _SOFT_KMEANS_ALGO_H

#include <map>
#include <fstream>

#include "AlgoBase.h"
#include "Utils/Progress.h"
#include "Utils/CoClusterParams.h"
#include "Utils/Mutex.h"
#include "Utils/Timer.h"


using std::map;
class SoftKMeansAlgoTest;

class SoftKMeansAlgo : public AlgoBase {
 friend class SoftKMeansAlgoTest;

  struct MaxSim {
    MaxSim() : node_id(0), sim(0.0) {}
    MaxSim(const uint32_t &nid, const double &s) : node_id(nid), sim(s) {}

    uint32_t node_id;
    double sim;
  };


 public:
  SoftKMeansAlgo(const CoClusterParams &params);
  ~SoftKMeansAlgo();

  bool ParallelStep();
  bool NeedMoreIter();
  bool NeedMoreColIter();


 private:
  void Init();
  void InitIter();
  void FinishIter();
  void Finish();

  void InitiateClusters();

  double SimpleSimilarity(uint32_t &node_id, uint32_t &min_clust,
                          vector<double> *sims,
                          std::ofstream* sims_result = NULL);
  double NormalizedSimilarity(uint32_t &node_id, uint32_t &min_clust,
                              std::ofstream* output_result = NULL);

  Mutex mutex_;
  uint32_t lines_;
  bool verbose_;
  bool need_more_col_iter_;

  uint32_t row_moved_items_;
  uint32_t col_moved_items_;
  Progress<uint32_t> *progress_;

  map<double, uint32_t> similarities_;
  vector<bool> empty_clusters_;

  vector<double> *mins;
  vector<double> *maxs;

};


#endif // _SOFT_KMEANS_ALGO_H
