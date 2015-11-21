#ifndef __MAHALANOBIS_SIMILARITY_H
#define __MAHALANOBIS_SIMILARITY_H

#include <vector>

#include "SimilarityBase.h"
#include "Utils/Mutex.h"
#include "Common/ClusterInfoCollection.h"

using std::vector;


class MahalanobisSimilarity : public SimilarityBase {
 public:
  MahalanobisSimilarity(Data<double> *data, bool do_bicluster) : SimilarityBase(data, do_bicluster) {}
  ~MahalanobisSimilarity();

  void Init();
  void InitIter(bool row_view);
  /*! Returns the similarity of a node to a cluster */
  double SimilarityToCluster(const uint32_t &node_id, const uint32_t &clust_id);
  double VectorSimilarityToCluster(const vector<double> &data_vector, const uint32_t &clust_id);
  /*! Returns the similarity of node to a bicluster */
  double SimilarityToBiCluster(const uint32_t &node_id, const uint32_t &clust_id);
  /*! Adds a node to a cluster. Handles the informations stored about the */
  /*! corresponding cluster */
  void AddNodeToCluster(const uint32_t &node_id, const uint32_t &clust_id);
  void AddVectorToCluster(const vector<double> &avg, const uint32_t &clust_id);
  void FinishIter();
  void Finish();

  void OutputCoClustInfo() {}

 private:
  /*! Inverts a matrix stored in an array of doubles */
  void InvertMatrix(double *Min, double *Mout, int actualsize);

  double *cov_matrix_;

  /*! Stores the new clustering informations. After the current iteration it */
  /*! will be the local_info_->clust_info_; */
  vector< L2ClusterInfo > *tmp_clust_info_;
  LocalInfo<L2ItemInfo, L2ClusterInfo, L2CoClusterInfo> *local_info_;

  Mutex data_mutex_;
};


#endif // __MAHALANOBIS_SIMILARITY_H
