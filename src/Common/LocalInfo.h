/*! LocalInfo stores informations about the different parts of the dataset. */
/*! Every partition of the dataset has its own similarity function and */
/*! LocalInfo. ItemInfo, ClusterInfo, CoClusterInfo have to be implemented */
/*! in the corresponding similarity fuction. */

#ifndef __LOCAL_INFO_H
#define __LOCAL_INFO_H

#include <stdint.h>
#include <vector>

#include "Data.h"

using std::vector;


template<class ItemInfo, class ClusterInfo, class CoClusterInfo>
class LocalInfo {
  friend class KLSimilarityTest;

 public:
  LocalInfo();
  ~LocalInfo();

  /*! Initialize the row and column infos */
  void Init(vector<ItemInfo>* row_item_info,
            vector<ItemInfo>* col_item_info,
            vector<ClusterInfo>* row_clust_info,
            vector<ClusterInfo>* col_clust_info,
            Data<CoClusterInfo>* co_cluster_info);

  /*! Returns infos about the given item */
  const ItemInfo& GetRowItemInfo(const uint32_t& row_id);
  const ItemInfo& GetColItemInfo(const uint32_t& col_id);

  /*! Returns infos about the given cluster */
  const ClusterInfo& GetRowClusterInfo(const uint32_t& row_clust_id);
  const ClusterInfo& GetColClusterInfo(const uint32_t& col_clust_id);

  /*! Returns infos about the given co-cluster */
  const CoClusterInfo& GetCoClusterInfo(const uint32_t& row_clust_id, const uint32_t& col_clust_id);

  /*! Clears the current clustering informations */
  void Clear();

  /*! Sets row_view_ attribute */
  void SetRowView(const bool &row_view);

  /*! Swaps the actual and the temporary item info */
  void SwapItemInfo(vector< ItemInfo >* tmp_item_info);
  /*! Swaps the actual and the temporary cluster info */
  void SwapClusterInfo(vector< ClusterInfo >* tmp_cluster_info);
  /*! Swaps the actual and the temporary co-cluster info */
  void SwapCoClusterInfo(Data< CoClusterInfo >* tmp_co_cluster_info);


 protected:
  /*! Storing infos about items */
  vector< ItemInfo > *row_item_info_;
  vector< ItemInfo > *col_item_info_;

  /*! Storing infos about clusters */
  vector< ClusterInfo > *row_clust_info_;
  vector< ClusterInfo > *col_clust_info_;

  /*! Storing infos about co-clusters */
  Data<CoClusterInfo> *co_cluster_info_;

  /*! Storing the corresponding partition of the dataset */
  Data<double>* data_;

  /*! If true then view point is from rows if false then */
  /*! view point is from columns */
  bool row_view_;
};

#include "LocalInfo.tcc"

#endif // LOCAL_INFO_H
