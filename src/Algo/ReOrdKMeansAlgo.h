#ifndef _REORD_KMEANS_ALGO_H
#define _REORD_KMEANS_ALGO_H

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
class ReOrdKMeansAlgoTest;

class ReOrdKMeansAlgo : public AlgoBase {
 friend class ReOrdKMeansAlgoTest;

  struct MaxSim {
    MaxSim() : node_id(0), sim(0.0) {}
    MaxSim(const uint32_t &nid, const double &s) : node_id(nid), sim(s) {}

    uint32_t node_id;
    double sim;
  };


 public:
  ReOrdKMeansAlgo(const CoClusterParams &params);
  ~ReOrdKMeansAlgo();

  bool ParallelStep();
  bool NeedMoreIter();
  bool NeedMoreColIter();


 private:
  void Init();
  void InitIter();
  void FinishIter();
  void Finish();

  void InitiateClusters();

  void SimpleSimilarity(uint32_t &node_id,
                          multimap<double, uint32_t> &sims_to_reorder);
  double ReordSimilarity(uint32_t &node_id, uint32_t &min_clust,
                         multimap<double, uint32_t> &sims_to_reorder);

  Mutex mutex_;
  uint32_t lines_;
  bool verbose_;
  bool need_more_col_iter_;

  uint32_t row_moved_items_;
  uint32_t col_moved_items_;
  Progress<uint32_t> *progress_;

  map<double, uint32_t> similarities_;
  DenseData<double> *all_similarities_;
  vector<bool> empty_clusters_;

  vector<double> *mins;
  vector<double> *maxs;

};


#endif // _REORD_KMEANS_ALGO_H
