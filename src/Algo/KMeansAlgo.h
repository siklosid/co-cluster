#ifndef _KMEANS_ALGO_H
#define _KMEANS_ALGO_H

#include <map>
#include <fstream>

#include "AlgoBase.h"
#include "Utils/Progress.h"
#include "Utils/CoClusterParams.h"
#include "Utils/Mutex.h"
#include "Utils/Timer.h"
#include "Common/DenseData.h"


using std::map;
using std::multimap;

class KMeansAlgoTest;

class KMeansAlgo : public AlgoBase {
 friend class KMeansAlgoTest;

  struct MaxSim {
    MaxSim() : node_id(0), sim(0.0) {}
    MaxSim(const uint32_t &nid, const double &s) : node_id(nid), sim(s) {}

    uint32_t node_id;
    double sim;
  };


 public:
  KMeansAlgo(const CoClusterParams &params);
  ~KMeansAlgo();

  bool ParallelStep();
  bool NeedMoreIter();
  bool NeedMoreColIter();


 protected:
  void Init();
  void InitIter();
  void FinishIter();
  void Finish();

  void InitiateClusters();

  double SimpleSimilarity(uint32_t &node_id, uint32_t &min_clust,
                          std::ofstream* sims_result = NULL);
  double NormalizedSimilarity(uint32_t &node_id, uint32_t &min_clust,
                              std::ofstream* output_result = NULL);
  double NormalizedSimilarity2(uint32_t &node_id, uint32_t &min_clust);
  double NormalizedSimilarity3(uint32_t &node_id, uint32_t &min_clust);

  Mutex mutex_;
  uint32_t lines_;
  bool verbose_;
  bool need_more_col_iter_;

  uint32_t row_moved_items_;
  uint32_t col_moved_items_;
  Progress<uint32_t> *progress_;

  multimap<double, uint32_t> similarities_;
  DenseData<double> *all_similarities_;
  vector<bool> empty_clusters_;

  vector<double> *mins;
  vector<double> *maxs;

  vector<int> row_empty_clustering_;
  vector<int> col_empty_clustering_;

};


#endif // _KMEANS_ALGO_H
