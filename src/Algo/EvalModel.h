/*! Implements model evaluation, where the model is an existing clustering and
    evaluation means that we place the new/test elements into the existing clusters. */


#ifndef _EVAL_MODEL_H
#define _EVAL_MODEL_H

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
class EvalModelTest;

class EvalModel : public AlgoBase {
 friend class EvalModelTest;

  struct MaxSim {
    MaxSim() : node_id(0), sim(0.0) {}
    MaxSim(const uint32_t &nid, const double &s) : node_id(nid), sim(s) {}

    uint32_t node_id;
    double sim;
  };


 public:
  EvalModel(const CoClusterParams &params);
  ~EvalModel();

  bool ParallelStep();
  bool NeedMoreIter();
  bool NeedMoreColIter();


 private:
  void Init();
  void InitIter();
  void FinishIter();
  void Finish();

  void InitiateClusters();

  uint32_t SimpleSimilarity(map<uint32_t, double> &data,
                        double &sim);
  double ReordSimilarity(vector<double> &data, uint32_t &min_clust,
                         multimap<double, uint32_t> &sims_to_reorder);


  bool ReadLineFromTest(std::ifstream *input, vector<double> &line);
  bool ReadLineFromTest(std::ifstream *input, map<uint32_t, double> &line);

  Mutex mutex_;
  uint32_t lines_;
  bool verbose_;
  bool need_more_col_iter_;

  vector<std::ifstream*> test_data_set_;

  uint32_t row_moved_items_;
  uint32_t col_moved_items_;
  Progress<uint32_t> *progress_;

  map<double, uint32_t> similarities_;
  DenseData<double> *all_similarities_;
  vector<bool> empty_clusters_;

  std::ofstream test_clustering_;

  map<uint32_t, long> id2file_pos_;

  struct CompRev {
    bool operator()(uint32_t a, uint32_t b) {
      return b < a;
    }
  };
  map<uint32_t, uint32_t, CompRev> line2id_;
};


#endif // _EVAL_MODEL_H
