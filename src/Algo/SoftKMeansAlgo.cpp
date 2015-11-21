#include "SoftKMeansAlgo.h"

#include <stdlib.h>
#include <time.h>

#include <map>

#include "Common/Environment.h"
#include "Common/DenseData.h"

using std::map;

#define GRANULITY 100


SoftKMeansAlgo::SoftKMeansAlgo(const CoClusterParams &params)
  : AlgoBase(params), verbose_(params.verbose) {

  lines_ = 0;
  need_more_col_iter_ = true;
}

SoftKMeansAlgo::~SoftKMeansAlgo() {
  log_status("Ending kmeans algo");
}


bool SoftKMeansAlgo::ParallelStep() {
  mutex_.Lock();
  if (lines_ == global_info_->GetNumRowItems()) {
    running_iter_ = false;
    mutex_.Unlock();
    return running_iter_;
  }
  uint32_t my_line = lines_;
  lines_++;
  if (verbose_) progress_->show(lines_, "Lines processed");
  mutex_.Unlock();

  double sim_sum = 0.0;
  uint32_t min_clust;
  vector<double> *sims = new vector<double>(global_info_->GetNumRowClusts(), 0.0);
  if (row_view_ && sims_.size() > 1) {
    sim_sum = NormalizedSimilarity(my_line, min_clust);
  } else {
    sim_sum = SimpleSimilarity(my_line, min_clust, sims);
  }

  mutex_.Lock();
  //empty_clusters_[min_clust]=false;
  //similarities_.insert(std::pair<double, uint32_t>(sim, my_line));
  if (sim_sum > 0) {
    for (uint32_t i = 0; i < global_info_->GetNumRowClusts(); i++) {
      double *weight;
      weight = new double((*sims)[i]/sim_sum);
      AddNodeToCluster(my_line, i, weight);
      delete weight;
    }
  }

//  if (min_clust != global_info_->GetRowClust(my_line)) {
//    if (row_view_) row_moved_items_++;
//    else col_moved_items_++;
//  }
  mutex_.Unlock();
  delete sims;
  return running_iter_;
}


void SoftKMeansAlgo::Init() {


  global_info_->Init(data_set_[0]->GetNumRows(), data_set_[0]->GetNumCols());
  for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
    (*it)->Init();
  }

  InitiateClusters();
  if (do_bicluster_) {
    row_view_ = false;
    InitiateClusters();
  }

}


void SoftKMeansAlgo::InitIter() {
  lines_ = 0;
  if (row_view_) {
    row_moved_items_ = 0;
    col_moved_items_ = 0;
  }
  running_iter_ = true;
  global_info_->InitIter(row_view_);
  if (row_view_) {
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
      (*it)->InitIter(row_view_);
    }
  } else {
    sims_[0]->InitIter(row_view_);
  }
  progress_ = new Progress<uint32_t>(global_info_->GetNumRowItems());
  similarities_.clear();
  empty_clusters_.clear();
  empty_clusters_.resize(global_info_->GetNumRowClusts(), true);
}


bool SoftKMeansAlgo::NeedMoreIter() {
//  log_info("Moved items in rows: %u", row_moved_items_);
//  log_info("Moved items in cols: %u", col_moved_items_);
//  double percent_row;
//  double percent_col;
//  if (row_view_) {
//    percent_row = row_moved_items_*100.0/global_info_->GetNumRowItems();
//    percent_col = col_moved_items_*100.0/global_info_->GetNumColItems();
//  } else {
//    percent_row = row_moved_items_*100.0/global_info_->GetNumColItems();
//    percent_col = col_moved_items_*100.0/global_info_->GetNumRowItems();
//  }
//  if (percent_col <= 1) need_more_col_iter_ = false;
//  if (percent_row <= 1 && percent_col <= 1) return false;
  return true;
}


bool SoftKMeansAlgo::NeedMoreColIter() {
  return need_more_col_iter_;
}


void SoftKMeansAlgo::FinishIter() {
//  map<double, uint32_t>::reverse_iterator it_sims = similarities_.rbegin();
//  for (uint32_t clust_id = 0; clust_id < empty_clusters_.size(); clust_id++) {
//    if (empty_clusters_[clust_id]) {
//      log_info("Found an empty cluster");
//      if (row_view_) {
//        for (vector< SimilarityBase* >::iterator it = sims_.begin();
//            it != sims_.end(); it++) {
//          (*it)->AddNodeToCluster(it_sims->second, clust_id);
//        }
//      } else {
//        sims_[0]->AddNodeToCluster(it_sims->second, clust_id);
//      }
//      it_sims++;
//    }
//  }

//  uint32_t num_clusts = global_info_->GetNumRowClusts();
//  srand(time(NULL));
//  for (uint32_t clust_id = 0; clust_id < empty_clusters_.size(); clust_id++) {
//    if (empty_clusters_[clust_id]) {
//      log_info("Found an empty cluster");
//      vector<double> avg(global_info_->GetNumColItems());
//      for (uint32_t col_num = 0; col_num < global_info_->GetNumColItems(); col_num++) {
//        double val = ((*maxs)[col_num] - (*mins)[col_num])/GRANULITY;
//        uint32_t random_number = rand()%GRANULITY;
//        // log_dbg("random number: %u", random_number);
//        avg[col_num] = (*mins)[col_num] + val/2 + random_number*val;
//      }
//      for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
//        (*it)->AddVectorToCluster(avg, clust_id);
//        //moved_items_++;
//      }
//    }
//  }

  global_info_->FinishIter();
  if (row_view_) {
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
      (*it)->FinishIter();
    }
  } else {
    sims_[0]->FinishIter();
  }
  delete progress_;
}


void SoftKMeansAlgo::Finish() {

  /*global_info_->InitIter(true);
  for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
    (*it)->InitIter(true);
  }

  string output_dir = Environment::get().params().output_dir;
  std::ofstream row_result((output_dir + Environment::get().params().output_row_result).c_str());
  if(row_result.fail()) {
    log_err("Input file: %s not found", (output_dir + Environment::get().params().output_row_result).c_str());
  }

//  std::ofstream sims_result((output_dir + Environment::get().params().output_sims).c_str());
//  if (sims_result.fail()) {
//    log_err("Input file: %s not found", (output_dir + Environment::get().params().output_sims).c_str());
//  }
//  bool do_output_sims = Environment::get().params().do_output_sims;

  global_info_->SetRowView(true);
  log_status("Writing out the row result:");
  for (uint32_t i = 0; i < global_info_->GetNumRowItems(); i++) {
    row_result << i << " " <<  global_info_->GetRowClust(i) << std::endl;
//    if (do_output_sims) {
//      uint32_t min_clust;
//      if (sims_.size() > 1) NormalizedSimilarity(i, min_clust, &sims_result);
//      else SimpleSimilarity(i, min_clust, NULL, &sims_result);
//    }
  }
  row_result.close();
//  sims_result.close();

  if (do_bicluster_) {
    std::ofstream col_result((output_dir + Environment::get().params().output_col_result).c_str());
    if(col_result.fail()) {
      log_err("Input file: %s not found", (output_dir + Environment::get().params().output_col_result).c_str());
    }

    log_status("Writing out the col result:");
    for (uint32_t i = 0; i < global_info_->GetNumColItems(); i++) {
      col_result << i << " " << global_info_->GetColClust(i) << std::endl;
    }
    col_result.close();
  }

  global_info_->FinishIter();
  for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
    (*it)->FinishIter();
  }

*/

  for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
    (*it)->Finish();
  }
}


void SoftKMeansAlgo::InitiateClusters() {

  // InitIter for similarities and global_info
  global_info_->InitIter(row_view_);
  if (row_view_) row_moved_items_ = global_info_->GetNumRowItems();
  else col_moved_items_ = global_info_->GetNumRowItems();
  if (row_view_) {
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
      (*it)->InitIter(row_view_);
    }
  } else {
    sims_[0]->InitIter(row_view_);
  }


  // Different kind of initial steps. Uncomment the one you would like to use
//  // random item in each cluster
//  uint32_t num_items = global_info_->GetNumRowItems();
//  srand(time(NULL));
//
//  for (uint32_t clust_num = 0; clust_num < global_info_->GetNumRowClusts(); clust_num++) {
//    uint32_t rand_item = rand()%num_items;
//    AddNodeToCluster(rand_item, clust_num);
//  }


  // each item in a random cluster
  uint32_t num_clusts = global_info_->GetNumRowClusts();
  srand(time(NULL));
  //srand(1);

  double *w = new double(1.0);
  for (uint32_t row_num = 0; row_num < global_info_->GetNumRowItems(); row_num++) {
    uint32_t rand_item = rand()%num_clusts;
    AddNodeToCluster(row_num, rand_item, w);
  }
  delete w;

  // k-interval
//  mins = new vector<double>(global_info_->GetNumColItems(), numeric_limits<double>::max());
//  maxs = new vector<double>(global_info_->GetNumColItems(), numeric_limits<double>::min());
//
//  for (uint32_t row_num = 0; row_num < global_info_->GetNumRowItems(); row_num++) {
//    for (uint32_t col_num = 0; col_num < global_info_->GetNumColItems(); col_num++) {
//      if (data_set_[0]->GetItem(row_num, col_num) < (*mins)[col_num]) {
//        (*mins)[col_num] = data_set_[0]->GetItem(row_num, col_num);
//      }
//      if (data_set_[0]->GetItem(row_num, col_num) > (*maxs)[col_num]) {
//        (*maxs)[col_num] = data_set_[0]->GetItem(row_num, col_num);
//      }
//    }
//    if (row_num%10000 == 0) {
//      log_info("line: %u", row_num);
//    }
//  }
//
//  uint32_t num_clusts = global_info_->GetNumRowClusts();
//  srand(time(NULL));
//
//  for (uint32_t clust_num = 0; clust_num < global_info_->GetNumRowClusts(); clust_num++) {
//    vector<double> avg(global_info_->GetNumColItems());
//    for (uint32_t col_num = 0; col_num < global_info_->GetNumColItems(); col_num++) {
//      double val = ((*maxs)[col_num] - (*mins)[col_num])/GRANULITY;
//       uint32_t random_number = rand()%GRANULITY;
//       log_dbg("random number: %u", random_number);
//      avg[col_num] = (*mins)[col_num] + val/2 + clust_num*val;
//    }
//    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
//      (*it)->AddVectorToCluster(avg, clust_num);
//      moved_items_++;
//    }
//  }

  // featuresum

//  map<double, vector<uint32_t> > sum_to_id;
//  sum_to_id.clear();
//  for (uint32_t row_num = 0; row_num < global_info_->GetNumRowItems(); row_num++) {
//    double sum = 0;
//    for (uint32_t col_num = 0; col_num < global_info_->GetNumColItems(); col_num++) {
//      sum += data_set_[0]->GetItem(row_num, col_num);
//    }
//
//    map<double, vector<uint32_t> >::iterator it;
//    if ((it = sum_to_id.find(sum)) != sum_to_id.end()) {
//      sum_to_id[sum].push_back(row_num);
//    } else {
//      sum_to_id[sum].clear();
//      sum_to_id[sum].push_back(row_num);
//    }
//  }
//  uint32_t num = 0;
//  uint32_t clust_num = 0;
//  double threshold = (double)global_info_->GetNumRowItems()/global_info_->GetNumRowClusts();
//  for (map<double, vector<uint32_t> >::iterator it = sum_to_id.begin(); it != sum_to_id.end(); ++it) {
//    for (vector<uint32_t>::iterator itv = (it->second).begin(); itv != (it->second).end(); ++itv) {
//      if (num >= (clust_num+1)*threshold) clust_num++;
//      AddNodeToCluster(*itv, clust_num);
//      num++;
//    }
//  }

  global_info_->FinishIter();
  if (row_view_) {
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
      (*it)->FinishIter();
    }
  } else {
    sims_[0]->FinishIter();
  }
}


double SoftKMeansAlgo::SimpleSimilarity(uint32_t &node_id, uint32_t &min_clust,
                                        vector<double> *sims,
                                        std::ofstream *sims_result) {

  double sim_sum = 0;
  for (uint32_t clust_id = 0; clust_id < global_info_->GetNumRowClusts(); clust_id++) {
    double sim = 0;
    if (do_bicluster_) sim = sims_[0]->SimilarityToBiCluster(node_id, clust_id);
    else sim = sims_[0]->SimilarityToCluster(node_id, clust_id);
    if (sims != NULL) (*sims)[clust_id] = sim;
    sim_sum += sim;

    if (sims_result != NULL) *sims_result << (!myisinf(sim)?1/sim:0) << " ";
  }
  if (sims_result != NULL) *sims_result << std::endl;
  return sim_sum;
}


double SoftKMeansAlgo::NormalizedSimilarity(uint32_t &node_id, uint32_t &min_clust,
                                        std::ofstream *sims_result) {
  DenseData<double> sims_to_normalize(global_info_->GetNumRowClusts(), sims_.size());
  sims_to_normalize.SetRowView(true);
  vector<double> normalization(sims_.size(), 0.0);

  for (uint32_t clust_id = 0; clust_id < global_info_->GetNumRowClusts(); clust_id++) {
    double sim = 0;
    uint32_t data = 0;
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
      if (do_bicluster_ && it == sims_.begin()) sim = (*it)->SimilarityToBiCluster(node_id, clust_id);
      else sim = (*it)->SimilarityToCluster(node_id, clust_id);
      sims_to_normalize.SetItem(clust_id, data, sim);
      // log_info("XXXsim: %f", sim);
      if (!myisnan(sim) && !myisinf(sim)) {
          normalization[data] += sim*sim;
          // log_info("XXXnormplus");
      }
      data++;
    }
  }

  min_clust = 0;
  double min_sim = numeric_limits<double>::infinity();
  for (uint32_t i = 0; i < global_info_->GetNumRowClusts(); ++i) {
    Data<double>::const_iterator_base *it = sims_to_normalize.begin(i);
    Data<double>::const_iterator_base *it_end = sims_to_normalize.end(i);
    uint32_t data = 0;
    double sim = 0.0;
    uint32_t avg = sims_.size();
    // log_info("XXX******************************************");
    for (;*it != *it_end; ++(*it)) {
      // log_info("XXXsim: %f", *it);
      double norm = sqrt(normalization[data]);
      if (myisnan(**it) || norm == 0) {
        avg--;
      } else if (myisinf(sim)) {
        data++;
        break;
      } else {
        sim += **it/norm;
      }
      data++;
    }
    delete it;
    delete it_end;
    // log_info("XXXavg: %d", avg);
    if (avg == 0 || myisinf(sim)) sim = numeric_limits<double>::infinity();
    else sim /= avg;

    // log_info("XXX=============================================");
    // log_info("XXXnormsim: %f", sim);

    if (sim < min_sim) {
      min_sim = sim;
      min_clust = i;
    }
    if (sims_result != NULL) *sims_result << (!myisinf(sim)?1/sim:0) << " ";
  }
  if (sims_result != NULL) *sims_result << std::endl;
  return min_sim;
}
