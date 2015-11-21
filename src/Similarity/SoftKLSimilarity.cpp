#include "SoftKLSimilarity.h"

#include <math.h>
#include <fstream>
#include <typeinfo>

#include "Common/DenseData.h"


SoftKLSimilarity::~SoftKLSimilarity() {
  log_status("Ending KL similarity");
  delete local_info_;
}


void SoftKLSimilarity::Init() {
  vector<SoftKLItemInfo> *row_item_info = NULL;
  vector<SoftKLItemInfo> *col_item_info = NULL;
  vector<KLClusterInfo> *row_cluster_info = NULL;
  vector<KLClusterInfo> *col_cluster_info = NULL;
  DenseData<KLCoClusterInfo> *co_cluster_info = NULL;

  if (do_bicluster_) {
    row_item_info = new vector<SoftKLItemInfo>(global_info_->GetNumRowItems(), SoftKLItemInfo(global_info_->GetNumRowClusts()));
    col_item_info = new vector<SoftKLItemInfo>(global_info_->GetNumColItems(), SoftKLItemInfo(global_info_->GetNumColClusts()));

    // Compute the item informations. Only have to do once.
    for (uint32_t row_id = 0; row_id < global_info_->GetNumRowItems(); ++row_id) {
      Data<double>::const_iterator_base *it = data_->begin(row_id);
      Data<double>::const_iterator_base *end = data_->end(row_id);
      uint32_t col_id;
      for (; *it != *end; ++(*it)) {
        col_id = it->GetID();
        (*row_item_info)[row_id].sum_ += **it;
        (*col_item_info)[col_id].sum_ += **it;
      }
      delete it;
      delete end;

    }
    for (uint32_t row_id = 0; row_id < global_info_->GetNumRowItems(); ++row_id) {
      (*row_item_info)[row_id].log_sum_ = log((*row_item_info)[row_id].sum_);
      log_dbg("For row %u sum=%f, log_sum=%f",
              row_id, (*row_item_info)[row_id].sum_,
              (*row_item_info)[row_id].log_sum_);
    }
    for (uint32_t col_id = 0; col_id < global_info_->GetNumColItems(); ++col_id) {
      (*col_item_info)[col_id].log_sum_ = log((*col_item_info)[col_id].sum_);
      log_dbg("For col %u sum=%f, log_sum=%f",
              col_id, (*col_item_info)[col_id].sum_,
              (*col_item_info)[col_id].log_sum_);

    }

    // Initiate the other informations
    row_cluster_info = new vector<KLClusterInfo>(global_info_->GetNumRowClusts(), KLClusterInfo(0));
    col_cluster_info = new vector<KLClusterInfo>(global_info_->GetNumColClusts(), KLClusterInfo(0));
    co_cluster_info = new DenseData<KLCoClusterInfo>(global_info_->GetNumRowClusts(),
        global_info_->GetNumColClusts());
  } else {
    row_cluster_info = new vector<KLClusterInfo>(global_info_->GetNumRowClusts(),
        KLClusterInfo(global_info_->GetNumColItems()));
    col_cluster_info = new vector<KLClusterInfo>(global_info_->GetNumColClusts(),
        KLClusterInfo(global_info_->GetNumRowItems()));
  }

  local_info_ = new LocalInfo<SoftKLItemInfo, KLClusterInfo, KLCoClusterInfo>();
  local_info_->Init(row_item_info, // row_item_info
                    col_item_info, // col_item_info
                    row_cluster_info, // row_clust_info
                    col_cluster_info, // col_clust_info
                    co_cluster_info); // co_cluster_info
}


void SoftKLSimilarity::InitIter(bool row_view) {
  local_info_->SetRowView(row_view);
  data_->SetRowView(row_view);
  if (do_bicluster_) {
    tmp_item_info_ = new vector<SoftKLItemInfo>(global_info_->GetNumRowItems(),
        SoftKLItemInfo(global_info_->GetNumRowClusts()));
    tmp_clust_info_ = new vector<KLClusterInfo>(global_info_->GetNumRowClusts(),
        KLClusterInfo(0));
    tmp_co_clust_info_ =
      new DenseData<KLCoClusterInfo>(row_view?global_info_->GetNumRowClusts():global_info_->GetNumColClusts(),
                                     row_view?global_info_->GetNumColClusts():global_info_->GetNumRowClusts());
    tmp_co_clust_info_->SetRowView(row_view);
  } else {
    tmp_clust_info_ = new vector<KLClusterInfo>(global_info_->GetNumRowClusts(),
        KLClusterInfo(global_info_->GetNumColItems()));
  }
}


double SoftKLSimilarity::SimilarityToCluster(const uint32_t &node_id,
    const uint32_t &clust_id) {

  log_dbg("Measuring the similarity of node: %u to cluster: %u", node_id, clust_id);
  double sim = 0.0;



  Data<double>::const_iterator_base *it = data_->begin(node_id);
  Data<double>::const_iterator_base* end = data_->end(node_id);

  uint32_t col_id = 0;
  for (; *it != *end; ++(*it)) {
    col_id = it->GetID();
    double left = **it;
    double right = local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id];
    log_dbg("row_id: %u, col_id: %u, left=%f, right=%f", node_id, col_id, left, right);
    if (left == 0) continue;
    if (right == 0) sim += pow(right/local_info_->GetRowClusterInfo(clust_id).num_elements_, 2);
    double sim_item = left*(log(left) - right);
    log_dbg("sim_item: %f", sim_item);
    sim += sim_item;
  }

  log_dbg("Similarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim);
  return sim;
}


double SoftKLSimilarity::SimilarityToBiCluster(const uint32_t &node_id,
    const uint32_t &clust_id) {


  log_dbg("Measuring the similarity of node: %u to cluster: %u", node_id, clust_id);
  double sim = 0.0;
  uint32_t col_clust = 0;
  //uint32_t num_col_items = global_info_->GetNumColItems();
  Data<double>::const_iterator_base *it = data_->begin(node_id);
  Data<double>::const_iterator_base *end = data_->end(node_id);

  uint32_t col_id;
  for (; *it != *end; ++(*it)) {
    col_id = it->GetID();
    for (col_clust = 0; col_clust < global_info_->GetNumColClusts(); col_clust++) {
      double weight;
      if ((weight = local_info_->GetColItemInfo(col_id).weights_[col_clust]) == 0.0) {
        log_dbg("Weight of item %u in column cluster %u is 0.0", col_id, col_clust);
        continue;
      }
      log_dbg("Weight of item %u in column cluster %u is %f", col_id, col_clust, weight);
      double data = **it;
      if (data == 0) {
        log_dbg("Item (%u, %u)=0", node_id, col_id);
        continue;
      }
      double kl_left = data/local_info_->GetRowItemInfo(node_id).sum_;
      double kl_right_dividend =
        log(data) +
        local_info_->GetRowClusterInfo(clust_id).log_sum_ +
        local_info_->GetColClusterInfo(col_clust).log_sum_;
      double kl_right_divisor =
        local_info_->GetCoClusterInfo(clust_id, col_clust).log_sum_ +
        local_info_->GetColItemInfo(col_id).log_sum_ +
        log(weight) +
        local_info_->GetRowItemInfo(node_id).log_sum_;

      log_dbg("row_clust_sum: %f, col_clust_sum: %f, co_clust_sum: %f, col_sum: %f, row_sum: %f",
              local_info_->GetRowClusterInfo(clust_id).log_sum_,
              local_info_->GetColClusterInfo(col_clust).log_sum_,
              local_info_->GetCoClusterInfo(clust_id, col_clust).log_sum_,
              local_info_->GetColItemInfo(col_id).log_sum_ + log(weight),
              local_info_->GetRowItemInfo(node_id).log_sum_);
      log_dbg("node_id: %u, col_id: %u\nkl_left=%f, kl_right_dividend=%f, kl_right_divisor=%f",
              node_id, col_id, kl_left, kl_right_dividend, kl_right_divisor);
      log_dbg("sim_item: %f", kl_left*(kl_right_dividend - kl_right_divisor));
      sim += kl_left*(kl_right_dividend - kl_right_divisor);
    }
  }
  delete it;
  delete end;
  log_dbg("XXXSimilarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim);
  return sim;
}


void SoftKLSimilarity::AddNodeToCluster(const uint32_t &node_id,
                                    const uint32_t &cluster_id) {
  log_dbg("Adding node: %u to cluster: %u", node_id, cluster_id);
  if (do_bicluster_) {
    (*tmp_clust_info_)[cluster_id].log_sum_ +=
      local_info_->GetRowItemInfo(node_id).sum_;
  } else {
    (*tmp_clust_info_)[cluster_id].num_elements_++;
  }

  Data<double>::const_iterator_base *it = data_->begin(node_id);
  Data<double>::const_iterator_base *end = data_->end(node_id);

  uint32_t col_id;
  for (; *it != *end; ++(*it)) {
    col_id = it->GetID();
    if (!do_bicluster_) {
      (*tmp_clust_info_)[cluster_id].col_sum_[col_id] += **it;
    } else {
      uint32_t col_clust = 0;
      if ((col_clust = global_info_->GetColClust(col_id)) == numeric_limits<uint32_t>::max()) {
        continue;
      }
      //tmp_co_clust_info_->GetItem(cluster_id, col_clust).log_sum_ +=
      //  **it;
    }
  }
  delete it;
  delete end;
}


void SoftKLSimilarity::AddNodeToCluster(const uint32_t &node_id,
                                    const uint32_t &cluster_id, double *weight) {

  log_dbg("Adding node: %u to cluster: %u, with weight: %f", node_id, cluster_id, *weight);
  if (do_bicluster_) {
    (*tmp_item_info_)[node_id].weights_[cluster_id] += *weight;
    (*tmp_clust_info_)[cluster_id].log_sum_ +=
      (*weight)*local_info_->GetRowItemInfo(node_id).sum_;
  } else {
    (*tmp_clust_info_)[cluster_id].num_elements_++;
  }


  Data<double>::const_iterator_base *it = data_->begin(node_id);
  Data<double>::const_iterator_base *end = data_->end(node_id);

  uint32_t col_id;
  for (; *it != *end; ++(*it)) {
    col_id = it->GetID();
    if (!do_bicluster_) {
      (*tmp_clust_info_)[cluster_id].col_sum_[col_id] += **it;
    } else {
      uint32_t col_clust = 0;
      for (col_clust = 0; col_clust < global_info_->GetNumColClusts(); ++col_clust) {
        double weight_col;
        if ((weight_col = local_info_->GetColItemInfo(col_id).weights_[col_clust]) == 0.0) {
          continue;
        }
        tmp_co_clust_info_->GetItem(cluster_id, col_clust).log_sum_ +=
          (*weight)*weight_col*(**it);
      }
    }
  }
  delete it;
  delete end;
}


void SoftKLSimilarity::AddVectorToCluster(const vector<double> &avg, const uint32_t &cluster_id) {
  log_dbg("Adding vector to cluster: %u", cluster_id);
}


void SoftKLSimilarity::FinishIter() {
  for (vector<KLClusterInfo>::iterator it = tmp_clust_info_->begin(); it != tmp_clust_info_->end(); ++it) {
    if (do_bicluster_) it->log_sum_ = log(it->log_sum_);
    else it->LogAvg();
  }
//  for (vector< vector< KLCoClusterInfo > >::iterator it = tmp_co_clust_info_->begin(); it != tmp_co_clust_info_->end(); ++it) {
//    for (vector<KLCoClusterInfo>::iterator it2 = it->begin(); it2 != it->end(); ++it2) {
//      it2->log_sum_ = log(it2->log_sum_);
//    }
//  }

  if (do_bicluster_) {
    for (uint32_t row_clust = 0; row_clust < global_info_->GetNumRowClusts(); ++row_clust) {
      for (uint32_t col_clust = 0; col_clust < global_info_->GetNumColClusts(); ++col_clust) {
        tmp_co_clust_info_->GetItem(row_clust, col_clust).log_sum_ =
          log(tmp_co_clust_info_->GetItem(row_clust, col_clust).log_sum_);
      }
    }

    for (uint32_t row_id = 0; row_id < global_info_->GetNumRowItems(); ++row_id) {
      (*tmp_item_info_)[row_id].sum_ = local_info_->GetRowItemInfo(row_id).sum_;
      (*tmp_item_info_)[row_id].log_sum_ = local_info_->GetRowItemInfo(row_id).log_sum_;
    }
  }

  local_info_->SwapItemInfo(tmp_item_info_);
  local_info_->SwapClusterInfo(tmp_clust_info_);
  if (do_bicluster_) local_info_->SwapCoClusterInfo(tmp_co_clust_info_);
}


void SoftKLSimilarity::Finish() {
  local_info_->SetRowView(true);
  string output_dir = Environment::get().params().output_dir;
  std::ofstream co_clusts_result((output_dir + Environment::get().params().output_co_clusts_result).c_str());

  if (co_clusts_result.fail()) {
    log_err("Couldn't open file: %s", (output_dir +
                                       Environment::get().params().output_co_clusts_result).c_str())
  }

  std::ofstream sims_result((output_dir + Environment::get().params().output_sims).c_str());
  if (sims_result.fail()) {
    log_err("Input file: %s not found", (output_dir + Environment::get().params().output_sims).c_str());
  }

  if (do_bicluster_ && Environment::get().params().do_output_co_clusts) {
    for (uint32_t i = 0; i < global_info_->GetNumRowClusts(); i++) {
      for (uint32_t j = 0; j < global_info_->GetNumColClusts(); j++) {
        co_clusts_result << local_info_->GetCoClusterInfo(i, j).log_sum_ -
        local_info_->GetRowClusterInfo(i).log_sum_ - local_info_->GetColClusterInfo(j).log_sum_ << " ";
      }
      co_clusts_result << std::endl;
    }
  }

  if (do_bicluster_ && Environment::get().params().do_output_sims) {
    for (uint32_t i = 0; i < global_info_->GetNumRowItems(); i++) {
      sims_result << local_info_->GetRowItemInfo(i).weights_[0];
      for (uint32_t col_clust = 1; col_clust < global_info_->GetNumColClusts(); col_clust++) {
        sims_result << " " << local_info_->GetRowItemInfo(i).weights_[col_clust];
      }
      sims_result << std::endl;
    }
  }

  co_clusts_result.close();
  sims_result.close();
}


