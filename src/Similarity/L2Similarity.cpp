#include "L2Similarity.h"

#include <math.h>

#include <fstream>


L2Similarity::~L2Similarity() {
  log_status("Ending L2 similarity");
  delete local_info_;
}


void L2Similarity::Init() {
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


void L2Similarity::InitIter(bool row_view) {

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


double L2Similarity::SimilarityToCluster(const uint32_t &node_id,
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
    double diff = **it - col_sum_avg;
    log_fdbg("Item: %f, col_sum_avg: %f, Diff: %f", **it, col_sum_avg, diff);
    sim += diff*diff;
  }
  delete it;
  delete end;

  sim = sqrt(sim);

  log_dbg("Similarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim);
  return sim;
}


double L2Similarity::VectorSimilarityToCluster(const vector<double> &data_vector,
                                         const uint32_t &clust_id) {

  double sim = 0.0;

  uint32_t num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_;
  if (num_elements == 0) {
    num_elements = 1;
    log_warn("Cluster is empty");
    return 0.0;
  }

//  Data<double>::const_iterator_base *it = data_->begin(node_id);
//  Data<double>::const_iterator_base *end = data_->end(node_id);



  for (uint32_t col_id = 0; col_id < data_vector.size(); ++col_id) {
    double col_sum_avg =
      local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id]/num_elements;
    // sim += pow(data_.GetItem(node_id, col_id) - col_sum_avg, 2);
    double diff = data_vector[col_id] - col_sum_avg;
    log_fdbg("Item: %f, col_sum_avg: %f, Diff: %f", data_vector[col_id], col_sum_avg, diff);
    sim += diff*diff;
  }
//  delete it;
//  delete end;

  sim = sqrt(sim);

  log_dbg("Similarity to clust %d is: %f", clust_id, sim);
  return sim;
}


double L2Similarity::SimilarityToBiCluster(const uint32_t &node_id,
                                           const uint32_t &clust_id) {

  log_dbg("Measuring the similarity of node: %u to cluster: %u", node_id, clust_id);
  double sim = 0.0;

  uint32_t num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_;
  if (num_elements == 0) {
    num_elements = 0;
    log_warn("Cluster is empty");
    return 0.0;
  }
  // This vector stores the similarities of the column clusters
  vector<double> sims(global_info_->GetNumColClusts(), 0.0);

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
    double diff = **it - col_sum_avg;
    log_fdbg("Item: %f, col_sum_avg: %f, Diff: %f", **it, col_sum_avg, diff);
    sims[global_info_->GetColClust(col_id)] += diff*diff;
  }
  delete it;
  delete end;
  for (uint32_t col_clust_id = 0; col_clust_id < sims.size(); ++col_clust_id) {
    sim += sqrt(sims[col_clust_id]);
  }
  sim /= global_info_->GetNumColClusts();
  log_dbg("Similarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim);
  return sim;
}


void L2Similarity::AddNodeToCluster(const uint32_t &node_id,
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


void L2Similarity::AddVectorToCluster(const vector<double> &avg, const uint32_t &cluster_id) {
  log_dbg("Adding vector to cluster: %u", cluster_id);
  (*tmp_clust_info_)[cluster_id].num_elements_++;
  for (uint32_t col_id = 0; col_id < data_->GetNumCols(); col_id++) {
    (*tmp_clust_info_)[cluster_id].col_sum_[col_id] += avg[col_id];
  }
}


void L2Similarity::FinishIter() {
  if (real_iter_) {
    local_info_->SwapClusterInfo(tmp_clust_info_);
    //if (do_bicluster_) local_info_->SwapCoClusterInfo(tmp_co_clust_info_);
  } else {
    delete tmp_clust_info_;
    //if (do_bicluster_) delete tmp_co_clust_info_;
  }
}


void L2Similarity::Finish() {
  if (Environment::get().params().do_output_clust_centers) {
    string output_dir = Environment::get().params().output_dir;
    std::ofstream clust_centers((output_dir + Environment::get().params().output_clust_centers).c_str());
    if (clust_centers.fail()) {
      log_err("Couldn't open file: %s", (output_dir + Environment::get().params().output_clust_centers).c_str());
    }

    log_status("Writing out the row cluster centers");
    local_info_->SetRowView(true);
    uint32_t num_clusts, num_cols;
    bool row_view = global_info_->GetRowView();
    if (row_view) {
      num_clusts = global_info_->GetNumRowClusts();
      num_cols = data_->GetNumCols();
    } else {
      num_clusts = global_info_->GetNumColClusts();
      num_cols = global_info_->GetNumRowItems();
    }
    for (uint32_t clust_id = 0; clust_id < num_clusts; clust_id++) {
      uint32_t num_elements = 0;
      if ((num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_) == 0) continue;
      for (uint32_t col_id = 0; col_id < num_cols; col_id++) {
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

