#ifndef __BREGMAN_SIMILARITY_H
#define __BREGMAN_SIMILARITY_H

#include "SimilarityBase.h"
#include "Utils/Mutex.h"
#include "Common/ClusterInfoCollection.h"
//#include "Common/DenseData.h"
//#include "Common/SparseData.h"



class BregmanSimilarity : public SimilarityBase {

 public:
  BregmanSimilarity(Data<double> *data, bool do_bicluster) : SimilarityBase(data, do_bicluster) {}
  ~BregmanSimilarity();

  void Init();
  void InitIter(bool row_view);
  /*! Returns the similarity of a node to a cluster */
  double SimilarityToCluster(const uint32_t &node_id, const uint32_t &clust_id);
  /*! Returns the similarity of node to a bicluster */
  double SimilarityToBiCluster(const uint32_t &node_id, const uint32_t &clust_id);
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
  vector< BregmanClusterInfo > *tmp_clust_info_;
  /*! Stores the new co-clustering informations. After the current iteration it */
  /*! will be the local_info_->co_clust_info_; */
  Data< BregmanCoClusterInfo> *tmp_co_clust_info_;

  /*! Stores all the needed informations of the corresponding data set */
  LocalInfo<BregmanItemInfo, BregmanClusterInfo, BregmanCoClusterInfo> *local_info_;

  bool real_iter;

  Mutex data_mutex_;
};


#endif // __BREGMAN_SIMILARITY_H

