#ifndef _SIMILARITY_BASE_H
#define _SIMILARITY_BASE_H

#include <map>

#include "Common/LocalInfo.h"
#include "Common/GlobalInfo.h"
#include "Common/Data.h"
#include "Common/DenseData.h"


using std::map;

class ItemInfo;
class ClusterInfo;
class CoClusterInfo;

class SimilarityBase {
  friend class KLSimilarityTest;

 public:
  SimilarityBase(Data<double> *data, bool do_bicluster);
  virtual ~SimilarityBase(){log_status("Ending simlarity base");}

  /*! Init function called before the algorithm starts */
  virtual void Init() = 0;

  /*! InitIter called before every iteration */
  virtual void InitIter(bool row_view) = 0;

  /*! Returns the similarity of a node to a cluster */
  virtual double SimilarityToCluster(const uint32_t &node_id, const uint32_t &clust_id) = 0;
  virtual double VectorSimilarityToCluster(const vector<double> &data_vector, const uint32_t &clust_id) { return 0.0; };
  /*! Returns the similarity of a node to a bi-cluster */
  virtual double SimilarityToBiCluster(const uint32_t &node_id, const uint32_t &clust_id) = 0;
  virtual double MapSimilarityToBiCluster(const map<uint32_t, double> &data_vector, const uint32_t &clust_id) { return 0.0; };

  /*! Adds a node to a cluster. Handles the informations stored about the */
  /*! corresponding cluster */
  virtual void AddNodeToCluster(const uint32_t &node_id, const uint32_t &clust_id) = 0;
  virtual void AddNodeToCluster(const uint32_t &node_id, const uint32_t &clust_id, double *weight) {};

  virtual void AddVectorToCluster(const vector<double> &avg, const uint32_t &clust_id) = 0;

  /*! FinishIter called after every iteration */
  virtual void FinishIter() = 0;

  /*! Finish called after the algorithm to do finalization */
  virtual void Finish() = 0;

  /*! Sets the last_iter_ variable */
  void SetLastIter(bool last_iter);

  /*! Outputs the mutual informations of co-clusters */
  virtual void OutputCoClustInfo() = 0;
  virtual void OutputOwnSim() {}

  static const GlobalInfo *global_info_;

 protected:
  vector< ItemInfo > *tmp_item_info_;
  vector< ClusterInfo > *tmp_clust_info_;
  vector< vector< CoClusterInfo > > *co_cluster_info_;

  Data<double> *data_;
  DenseData<double> *co_clusts_;

  bool do_bicluster_;
  bool real_iter_;
  bool last_iter_;

 private:
  LocalInfo<ItemInfo, ClusterInfo, CoClusterInfo> *local_info_;

};


#endif // _SIMILARITY_BASE_H
