#include "JSSimilarity.h"

#include <math.h>
#include <fstream>
#include <typeinfo>

#include "Common/DenseData.h"


JSSimilarity::~JSSimilarity() {
  log_status("Ending JS similarity");
  if (Environment::get().params().do_output_co_clusts) {
    delete co_clusts_;
  }
  delete local_info_;
}


void JSSimilarity::Init() {
  vector<JSItemInfo> *row_item_info = NULL;
  vector<JSItemInfo> *col_item_info = NULL;
  vector<JSClusterInfo> *row_cluster_info = NULL;
  vector<JSClusterInfo> *col_cluster_info = NULL;
  DenseData<JSCoClusterInfo> *co_cluster_info = NULL;

  if (do_bicluster_) {
    row_item_info = new vector<JSItemInfo>(global_info_->GetNumRowItems());
    col_item_info = new vector<JSItemInfo>(data_->GetNumCols());

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
    for (uint32_t col_id = 0; col_id < data_->GetNumCols(); ++col_id) {
      (*col_item_info)[col_id].log_sum_ = log((*col_item_info)[col_id].sum_);
      log_dbg("For col %u sum=%f, log_sum=%f",
              col_id, (*col_item_info)[col_id].sum_,
              (*col_item_info)[col_id].log_sum_);

    }

    // Initiate the other informations
    row_cluster_info = new vector<JSClusterInfo>(global_info_->GetNumRowClusts(), JSClusterInfo(0));
    col_cluster_info = new vector<JSClusterInfo>(global_info_->GetNumColClusts(), JSClusterInfo(0));
    co_cluster_info = new DenseData<JSCoClusterInfo>(global_info_->GetNumRowClusts(),
        global_info_->GetNumColClusts());
  } else {
    row_cluster_info = new vector<JSClusterInfo>(global_info_->GetNumRowClusts(),
        JSClusterInfo(data_->GetNumCols()));
    col_cluster_info = new vector<JSClusterInfo>(global_info_->GetNumColClusts(),
        JSClusterInfo(global_info_->GetNumRowItems()));
  }

  local_info_ = new LocalInfo<JSItemInfo, JSClusterInfo, JSCoClusterInfo>();
  local_info_->Init(row_item_info, // row_item_info
                    col_item_info, // col_item_info
                    row_cluster_info, // row_clust_info
                    col_cluster_info, // col_clust_info
                    co_cluster_info); // co_cluster_info
}


void JSSimilarity::InitIter(bool row_view) {
  real_iter = false;
  local_info_->SetRowView(row_view);
  data_->SetRowView(row_view);
  if (do_bicluster_) {
    tmp_clust_info_ = new vector<JSClusterInfo>(global_info_->GetNumRowClusts(), JSClusterInfo(0));
    tmp_co_clust_info_ =
      new DenseData<JSCoClusterInfo>(row_view?global_info_->GetNumRowClusts():global_info_->GetNumColClusts(),
                                     row_view?global_info_->GetNumColClusts():global_info_->GetNumRowClusts());
    tmp_co_clust_info_->SetRowView(row_view);
  } else {
    tmp_clust_info_ = new vector<JSClusterInfo>(global_info_->GetNumRowClusts(),
        JSClusterInfo(data_->GetNumCols()));
  }


  // Compute PreSim for row clusters
  for (uint32_t i = 0; i < global_info_->GetNumRowClusts(); ++i) {
    double pre_sim = 0.0;
    if (do_bicluster_) {
      log_dbg("PreSim for cluster %u is %f", i, LOG2/2);
      const_cast<JSClusterInfo&>(local_info_->GetRowClusterInfo(i)).SetPreSim(LOG2/2);
      continue;
    } else {
      for (uint32_t j = 0; j < data_->GetNumCols(); j++) {
        if (local_info_->GetRowClusterInfo(i).num_elements_ > 0) {
          pre_sim += local_info_->GetRowClusterInfo(i).col_sum_[j]/
                     local_info_->GetRowClusterInfo(i).num_elements_;
        }
      }
    }
    log_fdbg("PreSim for cluster %u is %f", i, pre_sim*LOG2/2);
    const_cast<JSClusterInfo&>(local_info_->GetRowClusterInfo(i)).SetPreSim(pre_sim*LOG2/2);
  }

  // If last iter and we need to output co_clust information ...
  if (last_iter_ && Environment::get().params().do_output_co_clusts) {
    co_clusts_ = new DenseData<double>(global_info_->GetNumRowClusts(),
                                       global_info_->GetNumColClusts(), 0.0);
  }
}


double JSSimilarity::SimilarityToCluster(const uint32_t &node_id,
    const uint32_t &clust_id) {

  log_dbg("Measuring the similarity of node: %u to cluster: %u", node_id, clust_id);
  double sim = local_info_->GetRowClusterInfo(clust_id).pre_sim_;

  Data<double>::const_iterator_base *it = data_->begin(node_id);
  Data<double>::const_iterator_base *end = data_->end(node_id);

  uint32_t col_id = 0;
  for (; *it != *end; ++(*it)) {
    col_id = it->GetID();
    double left = **it;
    if (left == 0) continue;

    double right = local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id]/
                   local_info_->GetRowClusterInfo(clust_id).num_elements_;
    log_dbg("row_id: %u, col_id: %u, left=%f, right=%f", node_id, col_id, left, right);
    double sim_item;
    if (right == 0) {
      sim_item = left*LOG2/2;
    } else {
      sim_item = (left*log(2*left/(right + left)) +
                  right*log(2*right/(right + left)))/2;

      // PreSim
      sim_item -= LOG2/2*local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id]/
                  local_info_->GetRowClusterInfo(clust_id).num_elements_;
    }
    log_dbg("sim_item: %f", sim_item);
    sim += sim_item;
  }

  delete it;
  delete end;

  log_dbg("Similarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim);
  return sim;
}


double JSSimilarity::SimilarityToBiCluster(const uint32_t &node_id,
    const uint32_t &clust_id) {

  log_dbg("Measuring the similarity of node: %u to cluster: %u", node_id, clust_id);
  double sim = local_info_->GetRowClusterInfo(clust_id).pre_sim_;
  log_dbg("Presim for clust %d is: %f", clust_id, sim);
  uint32_t col_clust = 0;

  // If given row cluster is empty
  if (local_info_->GetRowClusterInfo(clust_id).sum_ == 0) {
    return sim;
  }

  Data<double>::const_iterator_base *it = data_->begin(node_id);
  Data<double>::const_iterator_base *end = data_->end(node_id);

  uint32_t col_id;
  //double diff = 0.0;
  for (; *it != *end; ++(*it)) {
    col_id = it->GetID();
    if ((col_clust = global_info_->GetColClust(col_id)) == numeric_limits<uint32_t>::max()) {
      log_dbg("Column: %u not claustered yet", col_id);
      continue;
    }
    double data = **it;
    double kl_right_divisor = 0;
    if (data != 0) {

      double kl_left = data/local_info_->GetRowItemInfo(node_id).sum_;
      double kl_right_dividend =
        log(2*data*
            local_info_->GetRowClusterInfo(clust_id).sum_*
            local_info_->GetColClusterInfo(col_clust).sum_);
      kl_right_divisor =
        log(data*
            local_info_->GetRowClusterInfo(clust_id).sum_*
            local_info_->GetColClusterInfo(col_clust).sum_ +
            local_info_->GetCoClusterInfo(clust_id, col_clust).sum_*
            local_info_->GetRowItemInfo(node_id).sum_*
            local_info_->GetColItemInfo(col_id).sum_);

      sim += kl_left*(kl_right_dividend - kl_right_divisor);
      sim -= LOG2/2*local_info_->GetCoClusterInfo(clust_id, col_clust).sum_*
             local_info_->GetColItemInfo(col_id).sum_/
             (local_info_->GetRowClusterInfo(clust_id).sum_*
              local_info_->GetColClusterInfo(col_clust).sum_);

//      if (last_iter_) {
//        diff += kl_left*(kl_right_dividend - kl_right_divisor) -
//                LOG2/2*local_info_->GetCoClusterInfo(clust_id, col_clust).sum_*
//                local_info_->GetColItemInfo(col_id).sum_/
//                (local_info_->GetRowClusterInfo(clust_id).sum_*
//                local_info_->GetColClusterInfo(col_clust).sum_);
//      }
    }

    if (local_info_->GetCoClusterInfo(clust_id, col_clust).sum_ != 0) {
      double kl_left =
        local_info_->GetCoClusterInfo(clust_id, col_clust).sum_*
        local_info_->GetColItemInfo(col_id).sum_/
        (local_info_->GetRowClusterInfo(clust_id).sum_*
         local_info_->GetColClusterInfo(col_clust).sum_);
      double kl_right_dividend =
        LOG2 + local_info_->GetRowItemInfo(node_id).log_sum_ +
        local_info_->GetCoClusterInfo(clust_id, col_clust).log_sum_ +
        local_info_->GetColItemInfo(col_id).log_sum_;
      if (data == 0) {
        kl_right_divisor =
          log(data*
              local_info_->GetRowClusterInfo(clust_id).sum_*
              local_info_->GetColClusterInfo(col_clust).sum_ +
              local_info_->GetCoClusterInfo(clust_id, col_clust).sum_*
              local_info_->GetRowItemInfo(node_id).sum_*
              local_info_->GetColItemInfo(col_id).sum_);
      }
      sim += kl_left*(kl_right_dividend - kl_right_divisor);
//      if (last_iter_) {
//        diff += kl_left*(kl_right_dividend - kl_right_divisor);
//      }
    }
//    if (last_iter_ && Environment::get().params().do_output_co_clusts) {
//        co_clusts_->SetItem(global_info_->GetRowClust(node_id), global_info_->GetColClust(col_id),
//                            co_clusts_->GetItem(global_info_->GetRowClust(node_id),
//                                                global_info_->GetColClust(col_id)) + diff);
//    }
  }
  delete it;
  delete end;
  log_dbg("XXXSimilarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim/2);
  return sim/2;
}


double JSSimilarity::MapSimilarityToBiCluster(const map<uint32_t, double> &data_vector,
    const uint32_t &clust_id) {

  double sim = local_info_->GetRowClusterInfo(clust_id).pre_sim_;
  if (sim != sim) log_info("Presim for clust %d is: %f", clust_id, sim);
  uint32_t col_clust = 0;

  // If given row cluster is empty
  if (local_info_->GetRowClusterInfo(clust_id).sum_ == 0) {
    return sim;
  }

  if (sim != sim) log_info("Presim for clust %d is: %f", clust_id, sim);

  double sum = 0.0;
  for (map<uint32_t, double>::const_iterator itv = data_vector.begin(); itv != data_vector.end(); ++itv) {
    sum += itv->second;
  }
  double log_sum = log(sum);

//  Data<double>::const_iterator_base *it = data_->begin(node_id);
//  Data<double>::const_iterator_base *end = data_->end(node_id);

  //map<uint32_t, double>::const_iterator end = data_vector.end();

  log_dbg("Start iterating on data");

  if (sim != sim) log_info("Presim for clust %d is: %f", clust_id, sim);

  uint32_t col_id = 0;
  uint32_t numberofitems = 0;
  for (map<uint32_t, double>::const_iterator it = data_vector.begin(); it != data_vector.end(); ++it) {
    ++numberofitems;
    if (sim != sim) {
      log_info("Presim for clust %d is: %f", clust_id, sim);
      log_info("XXXnumberofitems %u", numberofitems);
    }
    col_id = it->first;
    if (local_info_->GetColItemInfo(col_id).sum_ == 0) continue;
    if (sim != sim) log_info("Presim for clust %d is: %f", clust_id, sim);
    if ((col_clust = global_info_->GetColClust(col_id)) == numeric_limits<uint32_t>::max()) {
      if (sim != sim) log_info("Presim for clust %d is: %f", clust_id, sim);
      log_dbg("Column: %u not claustered yet", col_id);
      continue;
    }
    if (sim != sim) log_info("Presim for clust %d is: %f", clust_id, sim);
    double data = it->second;
    double kl_right_divisor = 0;
    if (data != 0) {

      double kl_left = data/sum;
      double kl_right_dividend =
        log(2*data*
            local_info_->GetRowClusterInfo(clust_id).sum_*
            local_info_->GetColClusterInfo(col_clust).sum_);
      kl_right_divisor =
        log(data*
            local_info_->GetRowClusterInfo(clust_id).sum_*
            local_info_->GetColClusterInfo(col_clust).sum_ +
            local_info_->GetCoClusterInfo(clust_id, col_clust).sum_*
            sum*
            local_info_->GetColItemInfo(col_id).sum_);

      if (sim != sim) log_info("XXXHIBA0 %u %u %f", col_id, col_clust, sum);
      sim += kl_left*(kl_right_dividend - kl_right_divisor);
      sim -= LOG2/2*local_info_->GetCoClusterInfo(clust_id, col_clust).sum_*
             local_info_->GetColItemInfo(col_id).sum_/
             (local_info_->GetRowClusterInfo(clust_id).sum_*
              local_info_->GetColClusterInfo(col_clust).sum_);
      if (sim != sim) log_info("XXXHIBA1: %f, %f, %f", kl_left, kl_right_dividend, kl_right_divisor);
    }

    if (local_info_->GetCoClusterInfo(clust_id, col_clust).sum_ != 0) {
      double kl_left =
        local_info_->GetCoClusterInfo(clust_id, col_clust).sum_*
        local_info_->GetColItemInfo(col_id).sum_/
        (local_info_->GetRowClusterInfo(clust_id).sum_*
         local_info_->GetColClusterInfo(col_clust).sum_);
      double kl_right_dividend =
        LOG2 + log_sum +
        local_info_->GetCoClusterInfo(clust_id, col_clust).log_sum_ +
        local_info_->GetColItemInfo(col_id).log_sum_;
      if (data == 0) {
        kl_right_divisor =
          log(data*
              local_info_->GetRowClusterInfo(clust_id).sum_*
              local_info_->GetColClusterInfo(col_clust).sum_ +
              local_info_->GetCoClusterInfo(clust_id, col_clust).sum_*
              sum*
              local_info_->GetColItemInfo(col_id).sum_);
      }
      sim += kl_left*(kl_right_dividend - kl_right_divisor);
      if (sim != sim) log_info("XXXHIBA2: %f, %f, %f, %f, %f, %f, %u", kl_left, kl_right_dividend, kl_right_divisor, log_sum,
                               local_info_->GetCoClusterInfo(clust_id, col_clust).log_sum_,
                               local_info_->GetColItemInfo(col_id).log_sum_, col_id);
    }
    if (last_iter_ && Environment::get().params().do_output_co_clusts) {
//      if (data != 0 && sum != 0 &&
//          local_info_->GetColItemInfo(col_id).sum_ != 0) {
//
//        co_clusts_->SetItem(global_info_->GetRowClust(node_id), global_info_->GetColClust(col_id),
//                            co_clusts_->GetItem(global_info_->GetRowClust(node_id),
//                                                global_info_->GetColClust(col_id)) +
//                            data*
//                            log(data/(sum*
//                                      local_info_->GetColItemInfo(col_id).sum_)));
//      }
    }
  }

  if (sim == 0) log_info("XXXSimilarity to clust %d is: %f", clust_id, sim);
  return sim/2;
}


void JSSimilarity::AddNodeToCluster(const uint32_t &node_id,
                                    const uint32_t &cluster_id) {

  real_iter = true;
  log_dbg("Adding node: %u to cluster: %u", node_id, cluster_id);
  if (do_bicluster_) {
    (*tmp_clust_info_)[cluster_id].sum_ +=
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
      tmp_co_clust_info_->GetItem(cluster_id, col_clust).sum_ += **it;
    }
  }
  delete it;
  delete end;
}


void JSSimilarity::AddVectorToCluster(const vector<double> &avg, const uint32_t &cluster_id) {
  log_dbg("Adding vector to cluster: %u", cluster_id);
}


void JSSimilarity::FinishIter() {
  if (do_bicluster_) {
    for (uint32_t row_clust = 0; row_clust < global_info_->GetNumRowClusts(); ++row_clust) {
      for (uint32_t col_clust = 0; col_clust < global_info_->GetNumColClusts(); ++col_clust) {
        tmp_co_clust_info_->GetItem(row_clust, col_clust).log_sum_ =
          log(tmp_co_clust_info_->GetItem(row_clust, col_clust).sum_);
      }
    }

    if (last_iter_ && Environment::get().params().do_output_co_clusts) {
      string output_dir = Environment::get().params().output_dir;
      std::ofstream co_clusts_result((output_dir + Environment::get().params().output_co_clusts_result).c_str());

      if (co_clusts_result.fail()) {
        log_err("Couldn't open file: %s", (output_dir +
                                           Environment::get().params().output_co_clusts_result).c_str())
      }


//      for (uint32_t i = 0; i < global_info_->GetNumRowClusts(); i++) {
//        co_clusts_result << log(local_info_->GetCoClusterInfo(i, 0).sum_/
//              (local_info_->GetRowClusterInfo(i).sum_*local_info_->GetColClusterInfo(0).sum_));
//        for (uint32_t j = 1; j < global_info_->GetNumColClusts(); j++) {
//          co_clusts_result << " " << log(local_info_->GetCoClusterInfo(i, j).sum_/
//              (local_info_->GetRowClusterInfo(i).sum_*local_info_->GetColClusterInfo(j).sum_));
//        }
//        co_clusts_result << std::endl;
//      }

      // Temporal hack
      for (uint32_t i = 0; i < global_info_->GetNumRowItems(); ++i) {
        co_clusts_result << local_info_->GetColClusterInfo(0).col_sum_[i];
        for (uint32_t j = 1; j < global_info_->GetNumColClusts(); ++j) {
          co_clusts_result << local_info_->GetColClusterInfo(j).col_sum_[i];
        }
        co_clusts_result << std::endl;
      }
      co_clusts_result.close();



    }
  }

  if (real_iter) {
    local_info_->SwapClusterInfo(tmp_clust_info_);
    if (do_bicluster_) local_info_->SwapCoClusterInfo(tmp_co_clust_info_);
  } else {
    delete tmp_clust_info_;
    if (do_bicluster_) delete tmp_co_clust_info_;
  }
}


void JSSimilarity::Finish() {
//  string output_dir = Environment::get().params().output_dir;
//  std::ofstream co_clusts_result((output_dir + Environment::get().params().output_co_clusts_result).c_str());
//
//  if (co_clusts_result.fail()) {
//    log_err("Couldn't open file: %s", (output_dir +
//                     Environment::get().params().output_co_clusts_result).c_str())
//  }
//
//  if (do_bicluster_ && Environment::get().params().do_output_co_clusts) {
//    for (uint32_t i = 0; i < global_info_->GetNumRowClusts(); i++) {
//      for (uint32_t j = 0; j < global_info_->GetNumColClusts(); j++) {
//        co_clusts_result << local_info_->GetCoClusterInfo(i, j).log_sum_/
//        (local_info_->GetRowClusterInfo(i).log_sum_ *
//         local_info_->GetColClusterInfo(j).log_sum_) << " ";
//      }
//      co_clusts_result << std::endl;
//    }
//  }
//  co_clusts_result.close();
}


void JSSimilarity::OutputCoClustInfo() {
//  string output_dir = Environment::get().params().output_dir;
//  std::ofstream co_clusts_result((output_dir + Environment::get().params().output_co_clusts_result).c_str());
//
//  if (co_clusts_result.fail()) {
//    log_err("Couldn't open file: %s", (output_dir +
//                                       Environment::get().params().output_co_clusts_result).c_str())
//  }
//
//  if (do_bicluster_ && Environment::get().params().do_output_co_clusts) {
//    for (uint32_t i = 0; i < global_info_->GetNumRowClusts(); i++) {
//      co_clusts_result << co_clusts_->GetItem(i, 0);
//      for (uint32_t j = 1; j < global_info_->GetNumColClusts(); j++) {
//        co_clusts_result << " " << co_clusts_->GetItem(i, j);
//      }
//      co_clusts_result << std::endl;
//    }
//  }
//  co_clusts_result.close();
}


