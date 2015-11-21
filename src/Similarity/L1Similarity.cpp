#include "L1Similarity.h"

#include <fstream>

#include <math.h>


L1Similarity::~L1Similarity() {
  log_status("Ending L1 similarity");
  delete local_info_;
}


void L1Similarity::Init() {
  local_info_ = new LocalInfo<L2ItemInfo, L2ClusterInfo, L2CoClusterInfo>();
  local_info_->Init(NULL, // row_item_info
                    NULL, // col_item_info
                    new vector<L2ClusterInfo>(global_info_->GetNumRowClusts(),
                                              L2ClusterInfo(data_->GetNumCols())), // row_clust_info
                    (do_bicluster_?
                      new vector<L2ClusterInfo>(global_info_->GetNumColClusts(),
                                                L2ClusterInfo(global_info_->GetNumRowItems())):
                      NULL), // col_clust_info
                    NULL); // co_cluster_info
}


void L1Similarity::InitIter(bool row_view) {

  log_dbg("InitIter: %s", row_view?"true":"false");
  real_iter_ = false;
  local_info_->SetRowView(row_view);
  data_->SetRowView(row_view);
  tmp_clust_info_ =
    new vector<L2ClusterInfo>(global_info_->GetNumRowClusts(),
                              L2ClusterInfo(data_->GetNumCols()));

  /*for (uint32_t clust_id = 0; clust_id < global_info_->GetNumRowClusts(); clust_id++) {
    if (local_info_->GetRowClusterInfo(clust_id).num_elements_ == 0) continue;
    (*tmp_clust_info_)[clust_id].num_elements_++;
    uint32_t num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_;
    for (uint32_t col_id = 0; col_id < global_info_->GetNumColItems(); col_id++) {
      (*tmp_clust_info_)[clust_id].col_sum_[col_id] +=
      local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id]/num_elements;
    }
  }*/
}


double L1Similarity::SimilarityToCluster(const uint32_t &node_id,
                                         const uint32_t &clust_id) {

  log_dbg("Measuring the similarity of node: %u to cluster: %u", node_id, clust_id);
  double sim = 0.0;

  uint32_t num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_;
  if (num_elements == 0) {
    num_elements = 1;
    log_warn("Cluster is empty");
    return 0.0;
  }

  Data<double>::const_iterator_base *it = data_->begin(node_id);
  Data<double>::const_iterator_base *end = data_->end(node_id);
  uint32_t col_id;
  for (; *it != *end; ++(*it)) {
    col_id = it->GetID();
    double col_sum_avg =
      local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id]/num_elements;
    // sim += pow(data_.GetItem(node_id, col_id) - col_sum_avg, 2);
    double diff = abs(**it - col_sum_avg);
    log_fdbg("Item: %f, col_sum_avg: %f, Diff: %f", **it, col_sum_avg, diff);
    sim += diff;
  }
  delete it;
  delete end;
  log_dbg("Similarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim);
  return sim;
}


double L1Similarity::SimilarityToBiCluster(const uint32_t &node_id,
                                           const uint32_t &clust_id) {

  log_dbg("Measuring the similarity of node: %u to cluster: %u", node_id, clust_id);
  double sim = 0.0;

  uint32_t num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_;
  if (num_elements == 0) {
    num_elements = 0;
    log_warn("Cluster is empty");
    return 0.0;
  }

  Data<double>::const_iterator_base *it = data_->begin(node_id);
  Data<double>::const_iterator_base *end = data_->end(node_id);
  uint32_t col_id;
  for (; *it != *end; ++(*it)) {
    col_id = it->GetID();
    // If column is not yet clustered we simply step to the next column
    if (global_info_->GetColClust(col_id) == numeric_limits<uint32_t>::max()) {
      continue;
    }
    double col_sum_avg =
      local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id]/num_elements;
    // sim += pow(data_.GetItem(node_id, col_id) - col_sum_avg, 2);
    double diff = abs(**it - col_sum_avg);
    log_fdbg("Item: %f, col_sum_avg: %f, Diff: %f", **it, col_sum_avg, diff);
    sim += diff;
  }
  delete it;
  delete end;

  sim /= global_info_->GetNumColClusts();
  log_dbg("Similarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim);
  return sim;
}


void L1Similarity::AddNodeToCluster(const uint32_t &node_id,
				    const uint32_t &cluster_id) {

  real_iter_ = true;
  log_dbg("Adding node: %u to cluster: %u", node_id, cluster_id);
  data_mutex_.Lock();
  (*tmp_clust_info_)[cluster_id].num_elements_++;

  Data<double>::const_iterator_base* it = data_->begin(node_id);
  Data<double>::const_iterator_base* end = data_->end(node_id);
  uint32_t col_id;
  for (; *it != *end; ++(*it)) {
    col_id = it->GetID();
    (*tmp_clust_info_)[cluster_id].col_sum_[col_id] += **it;
  }
  delete it;
  delete end;
  data_mutex_.Unlock();
}


void L1Similarity::AddVectorToCluster(const vector<double> &avg, const uint32_t &cluster_id) {
  log_dbg("Adding vector to cluster: %u", cluster_id);
  (*tmp_clust_info_)[cluster_id].num_elements_++;
  for (uint32_t col_id = 0; col_id < data_->GetNumCols(); col_id++) {
    (*tmp_clust_info_)[cluster_id].col_sum_[col_id] += avg[col_id];
  }
}


void L1Similarity::FinishIter() {
  if (real_iter_) {
    local_info_->SwapClusterInfo(tmp_clust_info_);
    //if (do_bicluster_) local_info_->SwapCoClusterInfo(tmp_co_clust_info_);
  } else {
    delete tmp_clust_info_;
    //if (do_bicluster_) delete tmp_co_clust_info_;
  }
}


void L1Similarity::Finish() {
  if (Environment::get().params().do_output_clust_centers) {
    string output_dir = Environment::get().params().output_dir;
    std::ofstream clust_centers((output_dir + Environment::get().params().output_clust_centers).c_str());
    if (clust_centers.fail()) {
      log_err("Couldn't open file: %s", (output_dir + Environment::get().params().output_clust_centers).c_str());
    }

    log_status("Writing out the row cluster centers");
    local_info_->SetRowView(true);
    for (uint32_t clust_id = 0; clust_id < global_info_->GetNumRowClusts(); clust_id++) {
      uint32_t num_elements = 0;
      if ((num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_) == 0) continue;
      for (uint32_t col_id = 0; col_id < data_->GetNumCols(); col_id++) {
        clust_centers << local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id]/num_elements << " ";
      }
      clust_centers << std::endl;
    }
  }

//  if (!Environment::get().params().do_bicluster) return;
//  log_status("Writing out the col cluster centers");
//  for (uint32_t clust_id = 0; clust_id < global_info_->GetNumColClusts(); clust_id++) {
//    uint32_t num_elements = 0;
//    if ((num_elements = local_info_->GetColClusterInfo(clust_id).num_elements_) == 0) continue;
//    fprintf(stdout, "%u", clust_id);
//    for (uint32_t col_id = 0; col_id < global_info_->GetNumRowItems(); col_id++) {
//      //fprintf(stdout, " %f", local_info_->GetColClusterInfo(clust_id).col_sum_[col_id]/num_elements);
//    }
//    fprintf(stdout, "\n");
//  }
}

