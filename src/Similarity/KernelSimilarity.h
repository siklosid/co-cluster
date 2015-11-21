#ifndef __KERNEL_SIMILARITY_H
#define __KERNEL_SIMILARITY_H

#include "SimilarityBase.h"
#include "Utils/Mutex.h"
#include "Common/ClusterInfoCollection.h"


class KernelSimilarity : public SimilarityBase {
 public:
  KernelSimilarity(Data<double> *data, bool do_bicluster) : SimilarityBase(data, do_bicluster) {}
  ~KernelSimilarity();

  void Init();
  void InitIter(bool row_view);

  /*! Returns the similarity of a node to a cluster */
  double SimilarityToCluster(const uint32_t &node_id, const uint32_t &clust_id);
  /*! Returns the similarity of node to a bicluster */
  double SimilarityToBiCluster(const uint32_t &node_id, const uint32_t &clust_id);

  double VectorSimilarityToCluster(const vector<double> &data, const uint32_t &clust_id);

  /*! Adds a node to a cluster. Handles the informations stored about the */
  /*! corresponding cluster */
  void AddNodeToCluster(const uint32_t &node_id, const uint32_t &clust_id);
  void AddVectorToCluster(const vector<double> &avg, const uint32_t &clust_id);

  void FinishIter();
  void Finish();

  void OutputCoClustInfo() {}

 private:
  /*! Stores the new clustering informations. After the current iteration it */
  /*! will be the local_info_->clust_info_; */
  vector< KernelClusterInfo > *tmp_clust_info_;
  LocalInfo<KernelItemInfo, KernelClusterInfo, KernelCoClusterInfo> *local_info_;

  Mutex data_mutex_;
};


#endif // __KERNEL_SIMILARITY_H
