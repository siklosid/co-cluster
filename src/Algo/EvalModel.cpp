#include "EvalModel.h"

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
#define NUM_REORD 20


EvalModel::EvalModel(const CoClusterParams &params)
  : AlgoBase(params), verbose_(params.verbose),
  test_clustering_((string(Environment::get().params().output_dir) + "test_clustering.txt").c_str()) {

  lines_ = 0;
  need_more_col_iter_ = true;

  for (uint32_t i = 0; i < Environment::get().params().num_datas; ++i) {
    std::ifstream* input = new std::ifstream((string(Environment::get().params().input_dir) +
        string(Environment::get().params().input_test_file[i])).c_str());


    if(i == 0) {
      uint32_t id;
      uint32_t prev_id = -1;
      double tmp;
      while (!input->eof()) {

        long pos = input->tellg();
        *input >> id;
        if (id != prev_id) {
          id2file_pos_[id] = pos;
          log_dbg("File position for id: %u, %ld", id, pos);
          prev_id = id;
        }
        *input >> id;
        *input >> tmp;
      }

      input->close();
      delete input;
      input = new std::ifstream((string(Environment::get().params().input_dir) +
        string(Environment::get().params().input_test_file[i])).c_str());
    }

    if (input->is_open()) {
      test_data_set_.push_back(input);
    } else {
      log_err("Couldn't open test file: %s", Environment::get().params().input_test_file[i]);
    }

  }


  if (!test_clustering_.is_open()) {
    log_err("Couldn't open test_clustering file");
  }

  std::ifstream line2id((string(Environment::get().params().output_dir) + "line2id.txt").c_str());

  if (!line2id.is_open()) {
    log_err("Couldn't open line2id file");
  }

  uint32_t id;
  uint32_t num;
  uint32_t total_num = 0;
  while (!line2id.eof()) {
    line2id >> num;
    line2id >> id;
    line2id_[total_num] = id;
    log_dbg("real_id for id: %u is %u", total_num, id);
    total_num += num;
  }

  line2id.close();

  if (Environment::get().params().do_output_sims) {
    Environment::get().params().need_last_iter = true;
  }
}

EvalModel::~EvalModel() {
  log_status("Ending evaluate algo");

  for (uint32_t i = 0; i < test_data_set_.size(); i++) {
    test_data_set_[i]->close();
    delete test_data_set_[i];
  }
}


bool EvalModel::ParallelStep() {
  log_dbg("Starting parallel step");
  mutex_.Lock();
  vector<double> data_l2;
  map<uint32_t, double> data_js;
  running_iter_ = ReadLineFromTest(test_data_set_[0], data_js);
  running_iter_ = ReadLineFromTest(test_data_set_[1], data_l2);
  uint32_t my_line = lines_;
  lines_++;
  mutex_.Unlock();
  if (!running_iter_) {
    return running_iter_;
  }

  double sim = 0.0;
  uint32_t min_clust;

  multimap<double, uint32_t> sims_to_reorder;
  sims_to_reorder.clear();

  SimpleSimilarity(data_js, sims_to_reorder);
  sim = ReordSimilarity(data_l2, min_clust, sims_to_reorder);
  log_dbg("Node belongs to cluster: %d", min_clust);

  mutex_.Lock();
  test_clustering_ << my_line << " " << min_clust << std::endl;
  mutex_.Unlock();

  return running_iter_;
}


void EvalModel::Init() {


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


void EvalModel::InitIter() {
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


bool EvalModel::NeedMoreIter() {
  if (iter_finished_ > 0) {
    return false;
  } else {
    return true;
  }
}


bool EvalModel::NeedMoreColIter() {
  return false;
}


void EvalModel::FinishIter() {
  if (!last_iter_) {
    map<double, uint32_t>::reverse_iterator it_sims = similarities_.rbegin();
    uint32_t num_empty_clusters = 0;
    for (uint32_t clust_id = 0;
         clust_id < empty_clusters_.size() && it_sims != similarities_.rend();
         clust_id++) {
      if (empty_clusters_[clust_id]) {
        num_empty_clusters++;
        if (row_view_) {
          for (vector< SimilarityBase* >::iterator it = sims_.begin();
               it != sims_.end(); it++) {
            (*it)->AddNodeToCluster(it_sims->second, clust_id);
          }
        } else {
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


void EvalModel::Finish() {

  global_info_->InitIter(true);
  for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
    (*it)->InitIter(true);
  }

  string output_dir = Environment::get().params().output_dir;
  std::ofstream row_result((output_dir + Environment::get().params().output_row_result).c_str());
  if(row_result.fail()) {
    log_err("Input file: %s not found", (output_dir + Environment::get().params().output_row_result).c_str());
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

    log_status("Writing out the col result:");
    for (uint32_t i = 0; i < global_info_->GetNumRowItems(); i++) {
      col_result << i << " " << global_info_->GetRowClust(i);
      if (do_output_own_sim) {
        col_result << " " << sims_[0]->SimilarityToBiCluster(i, global_info_->GetRowClust(i)) << std::endl;
      } else {
        col_result << std::endl;
      }
    }
    col_result.close();


    global_info_->FinishIter();
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
      (*it)->FinishIter();
    }
  }



  for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
    (*it)->Finish();
  }
}


void EvalModel::InitiateClusters() {

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


  global_info_->FinishIter();
  if (row_view_) {
    for (vector< SimilarityBase* >::iterator it = sims_.begin(); it != sims_.end(); it++) {
      (*it)->FinishIter();
    }
  } else {
    sims_[0]->FinishIter();
  }
}


void EvalModel::SimpleSimilarity(map<uint32_t, double> &data,
                                 multimap<double, uint32_t> &sims_to_reorder) {

  for (uint32_t clust_id = 0; clust_id < global_info_->GetNumRowClusts(); clust_id++) {
    double sim = 0;
    sim = sims_[0]->MapSimilarityToBiCluster(data, clust_id);
    sims_to_reorder.insert(std::pair<double, uint32_t>(sim, clust_id));
  }
}


double EvalModel::ReordSimilarity(vector<double> &data, uint32_t &min_clust,
                                  multimap<double, uint32_t> &sims_to_reorder) {

  min_clust = 0;
  double min_sim = numeric_limits<double>::infinity();

  multimap<double, uint32_t>::iterator it;
  uint32_t num_reord;
  for (it = sims_to_reorder.begin(), num_reord = 0;
       it != sims_to_reorder.end() && num_reord < NUM_REORD;
       ++it, ++num_reord) {
    double sim = sims_[1]->VectorSimilarityToCluster(data, it->second);
    if (sim < min_sim) {
      min_sim = sim;
      min_clust = it->second;
    }

  }

  return min_sim;
}



bool EvalModel::ReadLineFromTest(std::ifstream *input, vector<double> &data_vector) {
  if (input->eof()) return false;

  string id;
  string line;
  double val;
  getline(*input, line);
  std::stringstream ss(line);
  ss >> id;
  while (!ss.eof()) {
    ss >> val;
    data_vector.push_back(val);
  }
  return true;
}


bool EvalModel::ReadLineFromTest(std::ifstream *input, map<uint32_t, double> &data_vector) {
  if (input->eof()) {
    log_dbg("Reached the end of file");
    return false;
  }

  string line;
  uint32_t id;
  uint32_t real_id;
  uint32_t col_id;
  double val;
  long pos;

  log_dbg("Line: %u, Lower bound: %u", lines_, (*(line2id_.lower_bound(lines_))).first);
  real_id = (*(line2id_.lower_bound(lines_))).second;
  log_dbg("Real id for line: %u is %u", lines_, real_id);

  if (id2file_pos_.find(real_id) == id2file_pos_.end()) {
    return true;
  } else {
    pos = id2file_pos_[real_id];
  }

  log_dbg("Position for id: %u is %ld", real_id, pos);
  input->seekg(pos);

  while (!input->eof()) {
    *input >> id;
    if (id != real_id || input->eof()) {
      break;
    }

    *input >> col_id;
    *input >> val;

    data_vector[col_id] = val;
    log_dbg("data[%u, %u]=%f", lines_, col_id, val);
  }
  return true;
}
