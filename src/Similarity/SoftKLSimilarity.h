#ifndef _SOFT_KL_SIMILARITY_H
#define _SOFT_KL_SIMILARITY_H

#include "SimilarityBase.h"
#include "Utils/Mutex.h"
#include "Common/ClusterInfoCollection.h"

template<class T>
class DenseData;


class SoftKLSimilarity : public SimilarityBase {

 public:
  SoftKLSimilarity(Data<double> *data, bool do_bicluster) : SimilarityBase(data, do_bicluster) {}
  ~SoftKLSimilarity();

  void Init();
  void InitIter(bool row_view);
  /*! Returns the similarity of a node to a cluster */
  double SimilarityToCluster(const uint32_t &node_id, const uint32_t &clust_id);
  /*! Returns the similarity of node to a bicluster */
  double SimilarityToBiCluster(const uint32_t &node_id, const uint32_t &clust_id);
  /*! Adds a node to a cluster. Handles the informations stored about the */
  /*! corresponding cluster */
  void AddNodeToCluster(const uint32_t &node_id, const uint32_t &clust_id);
  void AddNodeToCluster(const uint32_t &node_id, const uint32_t &clust_id, double *weight);
  void AddVectorToCluster(const vector<double> &avg, const uint32_t &clust_id);

  void FinishIter();
  void Finish();

  /*! Outputs the mutual informations of the co-clusters */
  /*! It have to be called before FinishIter */
  void OutputCoClustInfo() {};

 private:
  /*! Stores the item informations. It will be the local_info_->item_info_ */
  /*! after the current iteration. */
  vector<SoftKLItemInfo> *tmp_item_info_;
  /*! Stores the new clustering informations. After the current iteration it */
  /*! will be the local_info_->clust_info_; */
  vector< KLClusterInfo > *tmp_clust_info_;
  /*! Stores the new co-clustering informations. After the current iteration it */
  /*! will be the local_info_->co_clust_info_; */
  DenseData< KLCoClusterInfo> *tmp_co_clust_info_;

  /*! Stores all the needed informations of the corresponding data set */
  LocalInfo<SoftKLItemInfo, KLClusterInfo, KLCoClusterInfo> *local_info_;

  Mutex data_mutex_;
};


#endif // _SOFT_KL_SIMILARITY_H

