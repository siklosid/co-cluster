#include "Common/Environment.h"


template< class ItemInfo, class ClusterInfo, class CoClusterInfo >
LocalInfo<ItemInfo, ClusterInfo, CoClusterInfo>::LocalInfo() :
    row_view_(true) {

}


template< class ItemInfo, class ClusterInfo, class CoClusterInfo >
LocalInfo<ItemInfo, ClusterInfo, CoClusterInfo>::~LocalInfo() {
  log_status("Ending local info");
  if (row_item_info_ != NULL) {
    row_item_info_->clear();
    delete row_item_info_;
  }

  if (col_item_info_ != NULL) {
    col_item_info_->clear();
    delete col_item_info_;
  }

  if (row_clust_info_ != NULL) {
    row_clust_info_->clear();
    delete row_clust_info_;
  }

  if (col_clust_info_ != NULL) {
    col_clust_info_->clear();
    delete col_clust_info_;
  }

  if (co_cluster_info_ != NULL) {
    delete co_cluster_info_;
  }
}


template< class ItemInfo, class ClusterInfo, class CoClusterInfo >
void LocalInfo<ItemInfo, ClusterInfo,
    CoClusterInfo>::Init(vector<ItemInfo>* row_item_info,
                         vector<ItemInfo>* col_item_info,
                         vector<ClusterInfo>* row_clust_info,
                         vector<ClusterInfo>* col_clust_info,
                         Data< CoClusterInfo >* co_cluster_info) {
  row_item_info_ = row_item_info;
  col_item_info_ = col_item_info;

  row_clust_info_ = row_clust_info;
  col_clust_info_ = col_clust_info;

  co_cluster_info_ = co_cluster_info;
}

template< class ItemInfo, class ClusterInfo, class CoClusterInfo >
const ItemInfo& LocalInfo<ItemInfo, ClusterInfo,
			  CoClusterInfo>::GetRowItemInfo(const uint32_t& row_id) {
  if ( row_view_ ) {
    return (*row_item_info_)[row_id];
  } else {
    return (*col_item_info_)[row_id];
  }
}


template< class ItemInfo, class ClusterInfo, class CoClusterInfo >
const ItemInfo& LocalInfo<ItemInfo, ClusterInfo,
			  CoClusterInfo>::GetColItemInfo(const uint32_t& col_id) {
  if ( row_view_ ) {
    return (*col_item_info_)[col_id];
  } else {
    return (*row_item_info_)[col_id];
  }
}


template< class ItemInfo, class ClusterInfo, class CoClusterInfo >
const ClusterInfo& LocalInfo<ItemInfo, ClusterInfo,
			     CoClusterInfo>::GetRowClusterInfo(const uint32_t& row_clust_id) {
  if ( row_view_ ) {
    return (*row_clust_info_)[row_clust_id];
  } else {
    return (*col_clust_info_)[row_clust_id];
  }
}


template< class ItemInfo, class ClusterInfo, class CoClusterInfo >
const ClusterInfo& LocalInfo<ItemInfo, ClusterInfo,
			     CoClusterInfo>::GetColClusterInfo(const uint32_t& col_clust_id) {
  if ( row_view_ ) {
    return (*col_clust_info_)[col_clust_id];
  } else {
    return (*row_clust_info_)[col_clust_id];
  }
}


template< class ItemInfo, class ClusterInfo, class CoClusterInfo >
const CoClusterInfo& LocalInfo<ItemInfo, ClusterInfo,
			       CoClusterInfo>::GetCoClusterInfo(const uint32_t& row_clust_id,
							 const uint32_t& col_clust_id) {

    return co_cluster_info_->GetItem(row_clust_id, col_clust_id);
}


template< class ItemInfo, class ClusterInfo, class CoClusterInfo >
void LocalInfo<ItemInfo, ClusterInfo,
			       CoClusterInfo>::SetRowView(const bool &row_view) {
  row_view_ = row_view;
  if (co_cluster_info_ != NULL) co_cluster_info_->SetRowView(row_view);
  // if (data_ != NULL) data_->SetRowView(row_view);
}


template<class ItemInfo, class ClusterInfo, class CoClusterInfo>
void LocalInfo<ItemInfo, ClusterInfo, CoClusterInfo>::
    SwapItemInfo(vector<ItemInfo>* tmp_item_info) {

  if (row_view_) {
    row_item_info_->clear();
    delete row_item_info_;
    row_item_info_ = tmp_item_info;
  } else {
    col_item_info_->clear();
    delete col_item_info_;
    col_item_info_ = tmp_item_info;
  }
  tmp_item_info = NULL;
}


template< class ItemInfo, class ClusterInfo, class CoClusterInfo>
void LocalInfo<ItemInfo, ClusterInfo, CoClusterInfo>::
    SwapClusterInfo(vector<ClusterInfo>* tmp_cluster_info) {

  // swap(tmp_cluster_info, row_clust_info_);
  if (row_view_) {
    row_clust_info_->clear();
    delete row_clust_info_;
    row_clust_info_ = tmp_cluster_info;
  } else {
    col_clust_info_->clear();
    delete col_clust_info_;
    col_clust_info_ = tmp_cluster_info;
  }
  tmp_cluster_info = NULL;
}


template< class ItemInfo, class ClusterInfo, class CoClusterInfo>
void LocalInfo<ItemInfo, ClusterInfo,CoClusterInfo>::
    SwapCoClusterInfo(Data<CoClusterInfo>* tmp_co_cluster_info) {

  // swap(tmp_cluster_info, row_clust_info_);
  delete co_cluster_info_;
  co_cluster_info_ = tmp_co_cluster_info;
  tmp_co_cluster_info = NULL;
}


template< class ItemInfo, class ClusterInfo, class CoClusterInfo>
void LocalInfo<ItemInfo, ClusterInfo,CoClusterInfo>::Clear() {
  /*if (row_view_) {
    row_clust_info_->clear();
    delete row_clust_info_;
    row_clust_info_ = NULL;
  } else {
    col_clust_info_->clear();
    delete row_clust_info_;
    col_clust_info_ = NULL;
  }*/
}
