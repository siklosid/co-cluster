#include "KMeansAlgo.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <map>
#include <fstream>
#include <sstream>

#include "Common/Environment.h"
//#include "Common/DenseData.h"

using std::map;

#define GRANULITY 100


KMeansAlgo::KMeansAlgo(const CoClusterParams &params)
  : AlgoBase(params), verbose_(params.verbose) {

  lines_ = 0;
  need_more_col_iter_ = true;

  if (Environment::get().params().do_output_sims) {
    Environment::get().params().need_last_iter = true;
  }
}

KMeansAlgo::~KMeansAlgo() {
  log_status("Ending kmeans algo");
}


bool KMeansAlgo::ParallelStep() {
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

  double sim = 0.0;
  uint32_t min_clust;
  if (row_view_ && sims_.size() > 1) {
    sim = NormalizedSimilarity3(my_line, min_clust);
  } else {
    sim = SimpleSimilarity(my_line, min_clust);
  }

  if (last_iter_) return running_iter_;
  mutex_.Lock();
  empty_clusters_[min_clust]=false;
  similarities_.insert(std::pair<double, uint32_t>(sim, my_line));
  AddNodeToCluster(my_line, min_clust);
  if (min_clust != global_info_->GetRowClust(my_line)) {
    if (row_view_) row_moved_items_++;
    else col_moved_items_++;
  }
  mutex_.Unlock();
  return running_iter_;
}


void KMeansAlgo::Init() {
  log_info("NumRows=%u, NumCols=%u", data_set_[0]->GetNumRows(), data_set_[0]->GetNumCols());
  global_info_->Init(data_set_[0]->GetNumRows(), data_set_[0]->GetNumCols());
  for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
    (*it)->Init();
  }

  InitiateClusters();
  if (do_bicluster_) {
    row_view_ = false;
    InitiateClusters();
  }
  ++iter_finished_;
}


void KMeansAlgo::InitIter() {
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

  if (last_iter_ && Environment::get().params().do_output_sims) {
    all_similarities_ = new DenseData<double>(global_info_->GetNumRowItems(), global_info_->GetNumRowClusts());
    all_similarities_->SetRowView(true);
  }
}


bool KMeansAlgo::NeedMoreIter() {
  log_info("Moved items in rows: %u", row_moved_items_);
  log_info("Moved items in cols: %u", col_moved_items_);
  double percent_row;
  double percent_col;
  if (row_view_) {
    percent_row = row_moved_items_*100.0/global_info_->GetNumRowItems();
    percent_col = col_moved_items_*100.0/global_info_->GetNumColItems();
  } else {
    percent_row = row_moved_items_*100.0/global_info_->GetNumColItems();
    percent_col = col_moved_items_*100.0/global_info_->GetNumRowItems();
  }
  double stop_criteria = Environment::get().params().stop_criteria;
  if (percent_col <= stop_criteria) need_more_col_iter_ = false;
  if (percent_row <= stop_criteria && percent_col <= stop_criteria) return false;
  return true;
}


bool KMeansAlgo::NeedMoreColIter() {
  return need_more_col_iter_;
}


void KMeansAlgo::FinishIter() {
  if (!last_iter_ && iter_finished_ > 0) {
    if (row_view_) {
      row_empty_clustering_.clear();
      row_empty_clustering_.resize(global_info_->GetNumRowItems(), -1);
    } else {
      col_empty_clustering_.clear();
      col_empty_clustering_.resize(global_info_->GetNumRowItems(), -1);
    }


    map<double, uint32_t>::reverse_iterator it_sims = similarities_.rbegin();
    uint32_t num_empty_clusters = 0;
    for (uint32_t clust_id = 0;
         clust_id < empty_clusters_.size() && it_sims != similarities_.rend();
         clust_id++) {
      if (empty_clusters_[clust_id]) {
        num_empty_clusters++;
        if (row_view_) {
          row_empty_clustering_[it_sims->second] = clust_id;
          for (vector< SimilarityBase* >::iterator it = sims_.begin();
               it != sims_.end(); it++) {
            (*it)->AddNodeToCluster(it_sims->second, clust_id);
          }
        } else {
          col_empty_clustering_[it_sims->second] = clust_id;
          sims_[0]->AddNodeToCluster(it_sims->second, clust_id);
        }
        it_sims++;
      }
    }
    if (num_empty_clusters > 0) log_info("Found %u empty clusters", num_empty_clusters);
  }



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


void KMeansAlgo::Finish() {

  global_info_->InitIter(true);
  for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
    (*it)->InitIter(true);
  }

  string output_dir = Environment::get().params().output_dir;
  std::ofstream row_result((output_dir + Environment::get().params().output_row_result).c_str());
  if(row_result.fail()) {
    log_err("Input file: %s not found", (output_dir + Environment::get().params().output_row_result).c_str());
  }

  std::ofstream row_result_empty((output_dir + Environment::get().params().output_row_result + ".empty").c_str());
  if(row_result_empty.fail()) {
    log_err("Input file: %s not found", (output_dir + Environment::get().params().output_row_result + ".empty").c_str());
  }


  std::ofstream sims_result((output_dir + Environment::get().params().output_sims).c_str());
  if (sims_result.fail()) {
    log_err("Input file: %s not found", (output_dir + Environment::get().params().output_sims).c_str());
  }
  bool do_output_sims = Environment::get().params().do_output_sims;
  bool do_output_own_sim = Environment::get().params().do_output_own_sim;

  global_info_->SetRowView(true);
  log_status("Writing out the row result:");
  for (uint32_t i = 0; i < global_info_->GetNumRowItems(); i++) {
    row_result << i << " " <<  global_info_->GetRowClust(i);
    if (row_empty_clustering_.size() > i && row_empty_clustering_[i] != -1) {
      row_result_empty << i << " " << row_empty_clustering_[i] << std::endl;
    }
    if (do_output_own_sim) {
      if (do_output_sims) {
        row_result << " " << all_similarities_->GetItem(i, global_info_->GetRowClust(i)) << std::endl;
      } else {
        row_result << " " << sims_[0]->SimilarityToBiCluster(i, global_info_->GetRowClust(i)) << std::endl;
      }
    } else {
      row_result << std::endl;
    }
    if (do_output_sims) {
      //uint32_t min_clust;
      //if (sims_.size() > 1) NormalizedSimilarity(i, min_clust, &sims_result);
      //else SimpleSimilarity(i, min_clust, &sims_result);
      sims_result << all_similarities_->GetItem(i, 0);
      for (uint32_t row_clust = 1; row_clust < global_info_->GetNumRowClusts(); row_clust++) {
        sims_result << " " << all_similarities_->GetItem(i, row_clust);
      }
      sims_result << std::endl;
    }
  }
  row_result.close();
  row_result_empty.close();
  sims_result.close();

  if (do_bicluster_) sims_[0]->OutputCoClustInfo();

  global_info_->FinishIter();
  for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
    (*it)->FinishIter();
  }


  if (do_bicluster_) {
    global_info_->InitIter(false);
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
      (*it)->InitIter(false);
    }

    std::ofstream col_result((output_dir + Environment::get().params().output_col_result).c_str());
    if(col_result.fail()) {
      log_err("Input file: %s not found", (output_dir + Environment::get().params().output_col_result).c_str());
    }

    std::ofstream col_result_empty((output_dir + Environment::get().params().output_col_result + ".empty").c_str());
    if(col_result_empty.fail()) {
      log_err("Input file: %s not found", (output_dir + Environment::get().params().output_col_result + ".empty").c_str());
    }


    log_status("Writing out the col result:");
    for (uint32_t i = 0; i < global_info_->GetNumRowItems(); i++) {
      col_result << i << " " << global_info_->GetRowClust(i);
      if (col_empty_clustering_.size() > i && col_empty_clustering_[i] != -1) {
        col_result_empty << i << " " << col_empty_clustering_[i] << std::endl;
      }
      if (do_output_own_sim) {
        col_result << " " << sims_[0]->SimilarityToBiCluster(i, global_info_->GetRowClust(i)) << std::endl;
      } else {
        col_result << std::endl;
      }
    }
    col_result.close();
    col_result_empty.close();


    global_info_->FinishIter();
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
      (*it)->FinishIter();
    }
  }



  for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
    (*it)->Finish();
  }
}


void KMeansAlgo::InitiateClusters() {

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

  if (Environment::get().params().do_continue) {


    string clust_file;
    if (row_view_) {
      clust_file = (string(Environment::get().params().output_dir) +
                    string(Environment::get().params().output_row_result)).c_str();
    } else {
      clust_file = (string(Environment::get().params().output_dir) +
                    string(Environment::get().params().output_col_result)).c_str();
    }

    std::ifstream in(clust_file.c_str());
    if (in.fail()) {
      log_err("Couldn't open file: %s", clust_file.c_str());
      exit(1);
    }

    string line;
    std::stringstream ss;

    uint32_t id;
    uint32_t clust;
    getline(in, line);
    for (uint32_t num_line = 0; num_line < global_info_->GetNumRowItems() && in.good(); num_line++) {
      if (line.size() > 0) {
        ss.clear();
        ss.str(line);
        ss >> id;
        ss >> clust;
        AddNodeToCluster(id, clust);
        getline(in, line);
      } else {
        log_err("Error in cluster file: %s at line: %u", clust_file.c_str(), num_line);
        exit(1);
      }
    }

    in.close();

    std::ifstream in_empty((clust_file + ".empty").c_str());
    if (in_empty.fail()) {
      log_err("Couldn't open file: %s", clust_file.c_str());
      exit(1);
    }

    getline(in_empty, line);
    while (in_empty.good()) {
      if (line.size() > 0) {
        ss.clear();
        ss.str(line);
        ss >> id;
        ss >> clust;
        if (row_view_) {
          for (vector< SimilarityBase* >::iterator it = sims_.begin();
               it != sims_.end(); it++) {
            (*it)->AddNodeToCluster(id, clust);
          }
        } else {
          sims_[0]->AddNodeToCluster(id, clust);
        }
        getline(in_empty, line);
      } else {
        log_err("Error in cluster file: %s at", (clust_file + ".empty").c_str());
        exit(1);
      }
    }
    in_empty.close();

  } else {






    // each item in a random cluster
    uint32_t num_clusts = global_info_->GetNumRowClusts();
    srand(time(NULL));
    //srand(1);

    for (uint32_t row_num = 0; row_num < global_info_->GetNumRowItems(); row_num++) {
      uint32_t rand_item = rand()%num_clusts;
      AddNodeToCluster(row_num, rand_item);
    }

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
  }
  global_info_->FinishIter();
  if (row_view_) {
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
      (*it)->FinishIter();
    }
  } else {
    sims_[0]->FinishIter();
  }
}


double KMeansAlgo::SimpleSimilarity(uint32_t &node_id, uint32_t &min_clust,
                                    std::ofstream *sims_result) {
  min_clust = 0;
  double min_sim = numeric_limits<double>::infinity();
  for (uint32_t clust_id = 0; clust_id < global_info_->GetNumRowClusts(); clust_id++) {
    double sim = 0;
    if (do_bicluster_) sim = sims_[0]->SimilarityToBiCluster(node_id, clust_id);
    else sim = sims_[0]->SimilarityToCluster(node_id, clust_id);
    if (sim < min_sim) {
      min_sim = sim;
      min_clust = clust_id;
    }
    //if (sims_result != NULL) *sims_result << (!myisinf(sim)?1/sim:0) << " ";
    if (last_iter_ && Environment::get().params().do_output_sims) {
      mutex_.Lock();
      all_similarities_->SetItem(node_id, clust_id, sim);
      mutex_.Unlock();
    }
  }
  //if (sims_result != NULL) *sims_result << std::endl;
  //log_info("XXXMinSim: %f", min_sim);
  return min_sim;
}


double KMeansAlgo::NormalizedSimilarity(uint32_t &node_id, uint32_t &min_clust,
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
    for (; *it != *it_end; ++(*it)) {
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


double KMeansAlgo::NormalizedSimilarity2(uint32_t &node_id, uint32_t &min_clust) {


  double min_sim = numeric_limits<double>::max();
  for (uint32_t clust_id = 0; clust_id < global_info_->GetNumRowClusts(); clust_id++) {
    double sim = 0;
    //uint32_t data = 0;
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
      double tmp_sim;
      if (do_bicluster_ && it == sims_.begin()) tmp_sim = (*it)->SimilarityToBiCluster(node_id, clust_id);
      else tmp_sim = (*it)->SimilarityToCluster(node_id, clust_id);

      if (!myisnan(sim) && !myisinf(sim)) {
        sim += tmp_sim;
      }
    }
    if (sim < min_sim) {
      min_sim = sim;
      min_clust = clust_id;
    }

    if (last_iter_ && Environment::get().params().do_output_sims) {
      mutex_.Lock();
      all_similarities_->SetItem(node_id, clust_id, sim);
      mutex_.Unlock();
    }
  }
  return min_sim;
}


double KMeansAlgo::NormalizedSimilarity3(uint32_t &node_id, uint32_t &min_clust) {
  DenseData<double> sims_to_normalize(global_info_->GetNumRowClusts(), sims_.size());
  sims_to_normalize.SetRowView(true);
  vector<double> maximums(sims_.size(), numeric_limits<double>::min());
  vector<double> minimums(sims_.size(), numeric_limits<double>::max());

  vector<double> weights(sims_.size(), 0.0);
  for (uint32_t i = 0; i < sims_.size(); ++i) {
    weights[i] = Environment::get().params().data_weight[i];
  }


  for (uint32_t clust_id = 0; clust_id < global_info_->GetNumRowClusts(); clust_id++) {
    double sim = 0;
    uint32_t data = 0;
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++, data++) {
      if (do_bicluster_ && data == 0) sim = (*it)->SimilarityToBiCluster(node_id, clust_id);
      else sim = (*it)->SimilarityToCluster(node_id, clust_id);
      sims_to_normalize.SetItem(clust_id, data, sim);
      //log_info("XXXsim: %f", sim);
      if (!myisnan(sim) && !myisinf(sim)) {
        if (sim > maximums[data]) maximums[data] = sim;
        if (sim < minimums[data]) minimums[data] = sim;
        // log_info("XXXnormplus");
      }
      //data++;
    }
  }

//  for (uint32_t i = 0; i < sims_.size(); ++i) {
//    log_info("For node: %u min[%u]=%f max[%u]=%f", node_id, i, minimums[i], i, maximums[i]);
//  }

  min_clust = 0;
  double min_sim = numeric_limits<double>::max();
  for (uint32_t i = 0; i < global_info_->GetNumRowClusts(); ++i) {
    Data<double>::const_iterator_base *it = sims_to_normalize.begin(i);
    Data<double>::const_iterator_base *it_end = sims_to_normalize.end(i);
    uint32_t data = 0;
    double sim = 0.0;
    double avg = 0.0;
    // log_info("XXX******************************************");
    for (; *it != *it_end; ++(*it), ++data) {
      // log_info("XXXsim: %f", *it);

      if (myisnan(**it)) {
        //log_info("Sim for data: %u, is nan", data);
//        if ((data == 0 && weights[data] != 0.0) || (data > 0 && weights[0] == 0)) {
//          sim = **it;
//        }
        if (data == 0) {
          sim = **it;
        }
      } else if (myisinf(**it)) {
//        if ((data == 0 && weights[data] != 0.0) || (data > 0 && weights[0] == 0)) {
//          sim = **it;
//        }
        if (data == 0) {
          sim = **it;
        }
        //log_info("Sim for data: %u, is inf", data);
        //log_info("XXXinf %u %u %u", data, node_id, i);
      } else {
        double scale = maximums[data]-minimums[data];
        if (scale == 0) {
            sim += (**it)*weights[data];
        } else {
            sim += (((**it) - minimums[data])/scale + minimums[data])*weights[data];
        }
        //log_info("data: %u, min: %f, max: %f, actual: %f, sim: %f", data, minimums[data], maximums[data], **it, sim);
        avg += weights[data];
      }
    }
    delete it;
    delete it_end;
    log_dbg("avg: %f", avg);
    //if (avg == 0 || myisinf(sim)) sim = numeric_limits<double>::infinity();
    //else sim /= avg;
    if (avg != 0.0) {
      sim /= avg;
    }
    //if (sim == 0) log_info("XXXavg: %f", avg);

    // log_info("XXX=============================================");
    log_dbg("normalized sim: %f", sim);

    if (sim < min_sim) {
      min_sim = sim;
      min_clust = i;
    }


    if (last_iter_ && Environment::get().params().do_output_sims) {
      mutex_.Lock();
      all_similarities_->SetItem(node_id, i, sim);
      mutex_.Unlock();
    }

    //if (sims_result != NULL) *sims_result << (!myisinf(sim)?1/sim:0) << " ";
  }
  //if (sims_result != NULL) *sims_result << std::endl;
  return min_sim;
}

