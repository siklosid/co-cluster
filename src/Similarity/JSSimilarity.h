#ifndef __JS_SIMILARITY_H
#define __JS_SIMILARITY_H

#include <map>

#include "SimilarityBase.h"
#include "Utils/Mutex.h"
#include "Common/ClusterInfoCollection.h"
//#include "Common/DenseData.h"
//#include "Common/SparseData.h"

using std::map;

class JSSimilarity : public SimilarityBase {

 public:
  JSSimilarity(Data<double> *data, bool do_bicluster) :
      SimilarityBase(data, do_bicluster),
      LOG2(log(2)) {

    Environment::get().params().need_last_iter = true;
  }
  ~JSSimilarity();

  void Init();
  void InitIter(bool row_view);
  /*! Returns the similarity of a node to a cluster */
  double SimilarityToCluster(const uint32_t &node_id, const uint32_t &clust_id);
  /*! Returns the similarity of node to a bicluster */
  double SimilarityToBiCluster(const uint32_t &node_id, const uint32_t &clust_id);
  double MapSimilarityToBiCluster(const map<uint32_t, double> &data_vector, const uint32_t &clust_id);

  /*! Adds a node to a cluster. Handles the informations stored about the */
  /*! corresponding cluster */
  void AddNodeToCluster(const uint32_t &node_id, const uint32_t &clust_id);
  void AddVectorToCluster(const vector<double> &avg, const uint32_t &clust_id);
  void FinishIter();
  void Finish();

  /*! Outputs the mutual informations of the co-clusters */
  /*! It have to be called before FinishIter */
  void OutputCoClustInfo();

 private:
  /*! Stores the new clustering informations. After the current iteration it */
  /*! will be the local_info_->clust_info_; */
  vector< JSClusterInfo > *tmp_clust_info_;
  /*! Stores the new co-clustering informations. After the current iteration it */
  /*! will be the local_info_->co_clust_info_; */
  Data< JSCoClusterInfo> *tmp_co_clust_info_;

  /*! Stores all the needed informations of the corresponding data set */
  LocalInfo<JSItemInfo, JSClusterInfo, JSCoClusterInfo> *local_info_;

  bool real_iter;
  double LOG2;

  Mutex data_mutex_;
};


#endif // __KL_SIMILARITY_H

