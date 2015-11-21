#include "KLConstSimilarity.h"

#include <math.h>
#include <fstream>

#include "Common/DenseData.h"


KLConstSimilarity::~KLConstSimilarity() {
  log_status("Ending KL similarity");
  delete local_info_;
}


void KLConstSimilarity::Init() {
  vector<KLConstItemInfo> *row_item_info = NULL;
  vector<KLConstItemInfo> *col_item_info = NULL;
  vector<KLConstClusterInfo> *row_cluster_info = NULL;
  vector<KLConstClusterInfo> *col_cluster_info = NULL;
  Data<KLCoClusterInfo> *co_cluster_info = NULL;

  if (do_bicluster_) {
    row_item_info = new vector<KLConstItemInfo>(global_info_->GetNumRowItems());
    col_item_info = new vector<KLConstItemInfo>(data_->GetNumCols());

    std::ifstream labels((string(Environment::get().params().input_dir) + "labels.txt").c_str());
    if (labels.fail()) {
      log_err("Valami WRONG");
      exit(1);
    }

    // Compute the item informations. Only have to do once.
    for (uint32_t row_id = 0; row_id < global_info_->GetNumRowItems(); ++row_id) {
      uint32_t s;
      labels >> s;
      State state;
      switch (s) {
        case 0:
          state = FALSE;
          break;
        case 1:
          state = TRUE;
          break;
        case 2:
          state = NEUTRAL;
          break;
        default:
          state = NEUTRAL;
          log_err("Valami WRONG");
      }
      (*row_item_info)[row_id].state_ = state;
      for (uint32_t col_id = 0; col_id < data_->GetNumCols(); ++col_id) {
        double data = data_->GetItem(row_id, col_id);
        (*row_item_info)[row_id].sum_ += data;
        (*col_item_info)[col_id].sum_ += data;
      }
    }
    labels.close();
    for (uint32_t row_id = 0; row_id < global_info_->GetNumRowItems(); ++row_id) {
      (*row_item_info)[row_id].log_sum_ = log((*row_item_info)[row_id].sum_);
      log_dbg("For row %u sum=%f, log_sum=%f",
              row_id, (*row_item_info)[row_id].sum_,
              (*row_item_info)[row_id].log_sum_);
    }
    for (uint32_t col_id = 0; col_id < data_->GetNumCols(); ++col_id) {
      (*col_item_info)[col_id].log_sum_ = log((*col_item_info)[col_id].sum_);
      log_dbg("For col %u sum=%f, log_sum=%f",
              col_id, (*col_item_info)[col_id].sum_,
              (*col_item_info)[col_id].log_sum_);

    }

    // Initiate the other informations
    row_cluster_info = new vector<KLConstClusterInfo>(global_info_->GetNumRowClusts(), KLConstClusterInfo(0));
    col_cluster_info = new vector<KLConstClusterInfo>(global_info_->GetNumColClusts(), KLConstClusterInfo(0));
    co_cluster_info = new DenseData<KLCoClusterInfo>(global_info_->GetNumRowClusts(),
        global_info_->GetNumColClusts());
  } else {
    row_cluster_info = new vector<KLConstClusterInfo>(global_info_->GetNumRowClusts(),
        KLConstClusterInfo(data_->GetNumCols()));
    col_cluster_info = new vector<KLConstClusterInfo>(global_info_->GetNumColClusts(),
        KLConstClusterInfo(global_info_->GetNumRowItems()));
  }

  local_info_ = new LocalInfo<KLConstItemInfo, KLConstClusterInfo, KLCoClusterInfo>();
  local_info_->Init(row_item_info, // row_item_info
                    col_item_info, // col_item_info
                    row_cluster_info, // row_clust_info
                    col_cluster_info, // col_clust_info
                    co_cluster_info); // co_cluster_info
}


void KLConstSimilarity::InitIter(bool row_view) {
  local_info_->SetRowView(row_view);
  data_->SetRowView(row_view);
  if (do_bicluster_) {
    tmp_clust_info_ = new vector<KLConstClusterInfo>(global_info_->GetNumRowClusts(), KLConstClusterInfo(0));
    tmp_co_clust_info_ =
      new DenseData<KLCoClusterInfo>(row_view?global_info_->GetNumRowClusts():global_info_->GetNumColClusts(),
                                row_view?global_info_->GetNumColClusts():global_info_->GetNumRowClusts());
    tmp_co_clust_info_->SetRowView(row_view);
  } else {
    tmp_clust_info_ = new vector<KLConstClusterInfo>(global_info_->GetNumRowClusts(),
        KLConstClusterInfo(data_->GetNumCols()));
  }
  for (uint32_t clust_id = 0; clust_id < global_info_->GetNumRowClusts(); clust_id++) {
    (*tmp_clust_info_)[clust_id].state_ = local_info_->GetRowClusterInfo(clust_id).state_;
  }
}


double KLConstSimilarity::SimilarityToCluster(const uint32_t &node_id,
    const uint32_t &clust_id) {

  log_dbg("Measuring the similarity of node: %u to cluster: %u", node_id, clust_id);
  double sim = 0.0;

  Data<double>::const_iterator_base* it = data_->begin(node_id);
  Data<double>::const_iterator_base* end = data_->end(node_id);
  for (uint32_t col_id = 0; it != end; ++col_id, ++(*it)) {
    double left = **it;
    double right = local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id];
    log_dbg("row_id: %u, col_id: %u, left=%f, right=%f", node_id, col_id, left, right);
    if (left == 0 || right == 0) continue;
    double sim_item = left*(log(left) - right);
    log_dbg("sim_item: %f", sim_item);
    sim += sim_item;
  }

  log_dbg("Similarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim);
  return sim;
}


double KLConstSimilarity::SimilarityToBiCluster(const uint32_t &node_id,
    const uint32_t &clust_id) {

  log_dbg("Measuring the similarity of node: %u to cluster: %u", node_id, clust_id);
  double sim = 0.0;
  uint32_t col_clust = 0;
  Data<double>::const_iterator_base* it = data_->begin(node_id);
  Data<double>::const_iterator_base* end = data_->end(node_id);

  for (uint32_t col_id = 0; it != end; ++col_id, ++(*it)) {
    if ((col_clust = global_info_->GetColClust(col_id)) == numeric_limits<uint32_t>::max()) {
      log_dbg("Column: %u not claustered yet", col_id);
      continue;
    }
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
      local_info_->GetRowItemInfo(node_id).log_sum_;

    log_dbg("row_clust_sum: %f, col_clust_sum: %f, co_clust_sum: %f, col_sum: %f, row_sum: %f",
            local_info_->GetRowClusterInfo(clust_id).log_sum_,
            local_info_->GetColClusterInfo(col_clust).log_sum_,
            local_info_->GetCoClusterInfo(clust_id, col_clust).log_sum_,
            local_info_->GetColItemInfo(col_id).log_sum_,
            local_info_->GetRowItemInfo(node_id).log_sum_);
    log_dbg("node_id: %u, col_id: %u\nkl_left=%f, kl_right_dividend=%f, kl_right_divisor=%f",
            node_id, col_id, kl_left, kl_right_dividend, kl_right_divisor);
    log_dbg("sim_item: %f", kl_left*(kl_right_dividend - kl_right_divisor));
    sim += kl_left*(kl_right_dividend - kl_right_divisor);
  }
  log_dbg("XXXSimilarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim);
  if (local_info_->GetRowClusterInfo(clust_id).state_ != NEUTRAL && local_info_->GetRowItemInfo(node_id).state_ != NEUTRAL
      && local_info_->GetRowClusterInfo(clust_id).state_ != local_info_->GetRowItemInfo(node_id).state_) {
    sim += 10;
  }
  return sim;
}


void KLConstSimilarity::AddNodeToCluster(const uint32_t &node_id,
                                    const uint32_t &cluster_id) {

  log_dbg("Adding node: %u to cluster: %u", node_id, cluster_id);
  if (do_bicluster_) {
    (*tmp_clust_info_)[cluster_id].log_sum_ +=
      local_info_->GetRowItemInfo(node_id).sum_;
  } else {
    (*tmp_clust_info_)[cluster_id].num_elements_++;
  }

  Data<double>::const_iterator_base* it = data_->begin(node_id);
  Data<double>::const_iterator_base* end = data_->end(node_id);

  for (uint32_t col_id = 0; it != end; ++col_id, ++(*it)) {
    if (!do_bicluster_) {
      (*tmp_clust_info_)[cluster_id].col_sum_[col_id] += **it;
    } else {
      uint32_t col_clust = 0;
      if ((col_clust = global_info_->GetColClust(col_id)) == numeric_limits<uint32_t>::max()) {
        continue;
      }
      tmp_co_clust_info_->GetItem(cluster_id, col_clust).log_sum_ += **it;
    }
  }

  if ((*tmp_clust_info_)[cluster_id].state_ == NEUTRAL) {
    log_dbg("state of cluster: %u set to %d", cluster_id, local_info_->GetRowItemInfo(node_id).state_);
    (*tmp_clust_info_)[cluster_id].state_ = local_info_->GetRowItemInfo(node_id).state_;
  }
}


void KLConstSimilarity::AddVectorToCluster(const vector<double> &avg, const uint32_t &cluster_id) {
  log_dbg("Adding vector to cluster: %u", cluster_id);
}


void KLConstSimilarity::FinishIter() {
  for (vector<KLConstClusterInfo>::iterator it = tmp_clust_info_->begin(); it != tmp_clust_info_->end(); ++it) {
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
  }

  local_info_->SwapClusterInfo(tmp_clust_info_);
  if (do_bicluster_) local_info_->SwapCoClusterInfo(tmp_co_clust_info_);
}


void KLConstSimilarity::Finish() {
  string output_dir = Environment::get().params().output_dir;
  std::ofstream co_clusts_result((output_dir + Environment::get().params().output_co_clusts_result).c_str());

  if (co_clusts_result.fail()) {
    log_err("Couldn't open file: %s", (output_dir +
                     Environment::get().params().output_co_clusts_result).c_str())
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
  co_clusts_result.close();
}

