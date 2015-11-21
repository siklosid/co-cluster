#ifndef _ALGO_BASE_H
#define _ALGO_BASE_H

#include <stdint.h>

#include "Utils/CoClusterParams.h"
#include "Utils/log.h"
#include "Utils/ThreadManager.h"
#include "Utils/Timer.h"
#include "Similarity/SimilarityBase.h"
#include "Similarity/SimilarityFactory.h"
#include "Common/Data.h"
#include "Reader/ReaderControl.h"

class ParallelComp;


class AlgoBase {
 public:
  AlgoBase(const CoClusterParams &params);
  virtual ~AlgoBase();

  /*! Called from the main program. Controls the whole running of the algorithm */
  void Start();
  /*! Called by the computing threads that are stored in par_comps_ vector. */
  /*! This function have to be implemented thread safe!!! */
  virtual bool ParallelStep() = 0;
  /*! Adds a node to a cluster. It calls global_info's and similarities's */
  /*! AddNodeToClutser functions. */
  void AddNodeToCluster(const uint32_t &node_id, const uint32_t &cluster_id, double *weight = NULL);

  /*! Sets the number of running threads. We can set that how much resources */
  /*! we want to use. */
  void SetNumThreads(const int &num_threads);
  /*! Sets the number of runnings threads to the original value */
  void ResumeThreads();


 protected:
  /*! Initializing the algorithm */
  virtual void Init() = 0;
  /*! Do some stuff before an iteration if needed */
  virtual void InitIter() = 0;
  /*! Run an iteration */
  void Iter();
  /*! The main cicle of an iter. If we are doing biclustering it's called */
  /*! two times. Once with row_view=true and once with row_view=false */
  void MainCicle();
  /*! Do some stuff after an iteration if needed */
  virtual void FinishIter() = 0;
  /*! Finalize the algorithm */
  virtual void Finish() = 0;

  /*! Returns that do we need more iteration or not */
  virtual bool NeedMoreIter();
  /*! Returns that do we need more iteration on the columns or not */
  virtual bool NeedMoreColIter();
  /*! Number of iterations that we already done */
  uint32_t iter_finished_;
  /*! Maximum number of iterations */
  uint32_t num_iter_;

  inline bool myisinf(double &v) {
    return (v == numeric_limits<double>::infinity() ||
            v == -numeric_limits<double>::infinity());
  }

  inline bool myisnan(double &v) {
    return v != v;
  }

  /*! This vector stores the difact_num_threads_ferent parts of the dataset */
  DataSet data_set_;
  /*! Reads the dataset into memory */
  ReaderControl reader_control_;
  /*! Stores the similarity functions for each dataset */
  vector< SimilarityBase* > sims_;
  /*! Stores the parallel computing threads */
  vector< ParallelComp* > par_comps_;
  /*! Stores global informations about clustering */
  GlobalInfo *global_info_;
  /*! Parallel threads communicates the end of an iteration through this variable */
  bool running_iter_;
  /*! Do biclustering or not. This value is set by SimilarityFactory */
  bool do_bicluster_;
  bool row_view_;

//  /*! If we need to do some parallel computing after the last iteration then */
//  /*! we have to set this true. */
//  bool need_last_iter_;
  /*! We are setting this true if we are after the last iteration and if need_last_iter_ */
  /*! iter is true. This way derived class will know that now comes the last iter */
  bool last_iter_;


 private:
  Timer timer;
  SimilarityFactory *similarity_factory_;
  int act_num_threads_;
};


/*! ParallelComp implements one thread of the computing threads. It calls */
/*! the Algo's ParallelStep function in a cicle until the iteration is finished */
class ParallelComp : public ThreadManager {
 public:
  ParallelComp(AlgoBase *algo);

  void Main();


 private:
  AlgoBase *_algo;

};

#endif // _ALGO_BASE_H
