#include "EvalKMeans.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <map>
#include <fstream>
#include <sstream>

#include "Common/Environment.h"
//#include "Common/DenseData.h"

using std::map;


EvalKMeans::EvalKMeans(const CoClusterParams &params)
  : KMeansAlgo(params),
    test_clustering_((string(Environment::get().params().output_dir) + "test_clustering.txt").c_str()) {

  for (uint32_t i = 0; i < Environment::get().params().num_datas; ++i) {
    std::ifstream* input = new std::ifstream((string(Environment::get().params().input_dir) +
        string(Environment::get().params().input_test_file[i])).c_str());


    if(i == 0) {
      string line;
      long pos = input->tellg();
      getline(*input, line);
      std::stringstream ss;


      uint32_t id;
      uint32_t prev_id = -1;
      double tmp;
      while (input->good()) {
        ss.clear();
        ss.str(line);
        ss >> id;
        if (id != prev_id) {
          id2file_pos_[id] = pos;
          log_dbg("File position for id: %u, %ld", id, pos);
          prev_id = id;
        }
        ss >> id;
        ss >> tmp;
        pos = input->tellg();
        getline(*input, line);
      }
      //  id2file_pos_[218] = 1495164;
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
}

EvalKMeans::~EvalKMeans() {
  log_status("Ending evaluate algo");

  for (uint32_t i = 0; i < test_data_set_.size(); i++) {
    test_data_set_[i]->close();
    delete test_data_set_[i];
  }
}


bool EvalKMeans::ParallelStep() {
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
  uint32_t min_clust = 0;

  vector<double> similarities(global_info_->GetNumRowClusts());

  sim = NormalizedSimilarity4(data_js, data_l2, min_clust, similarities);
  log_dbg("Node belongs to cluster: %d", min_clust);

  mutex_.Lock();
  test_clustering_ << my_line << " " << min_clust;
  for (uint32_t i = 0; i < similarities.size(); ++i) {
    test_clustering_ << " " << similarities[i];
  }
  test_clustering_ << std::endl;
//  for (map<uint32_t, double>::iterator it = data_js.begin(); it != data_js.end(); ++it) {
//    test_clustering_ << " " << it->first;
//  }
//  test_clustering_ << std::endl;
  mutex_.Unlock();

  return running_iter_;
}


bool EvalKMeans::NeedMoreIter() {
  if (iter_finished_ > 1) {
    return false;
  } else {
    return true;
  }
}


bool EvalKMeans::NeedMoreColIter() {
  return false;
}


bool EvalKMeans::ReadLineFromTest(std::ifstream *input, vector<double> &data_vector) {
  if (!input->good()) return false;

  uint32_t id = 0;
  string line;
  double val;
  getline(*input, line);
  std::stringstream ss(line);
  //ss >> id;
  while (!ss.eof()) {
    ss >> val;
    data_vector.push_back(val);
    log_fdbg("data[%u, %u]=%f", lines_, id, val);
    ++id;
  }
  return true;
}


bool EvalKMeans::ReadLineFromTest(std::ifstream *input, map<uint32_t, double> &data_vector) {

  if (input->eof()) {
    log_info("Reached the end of file");
    return false;
  }

  string line;
  uint32_t id;
  uint32_t real_id;
  uint32_t col_id;
  double val;
  long pos;


  real_id = lines_;


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
    log_fdbg("data[%u, %u]=%f", lines_, col_id, val);
  }
  return true;
}


double EvalKMeans::NormalizedSimilarity4(map<uint32_t, double> &data_js,
                                         vector<double> &data_l2, uint32_t &min_clust,
                                         vector<double> &similarities) {
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
      if (do_bicluster_ && data == 0) sim = (*it)->MapSimilarityToBiCluster(data_js, clust_id);
      else sim = (*it)->VectorSimilarityToCluster(data_l2, clust_id);
      sims_to_normalize.SetItem(clust_id, data, sim);
      //if (data == 0) log_info("XXXsim: %f", sim);
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

    similarities[i] = sim;
//    if (last_iter_ && Environment::get().params().do_output_sims) {
//      mutex_.Lock();
//      //all_similarities_->SetItem(node_id, i, sim);
//      mutex_.Unlock();
//    }

    //if (sims_result != NULL) *sims_result << (!myisinf(sim)?1/sim:0) << " ";
  }
  //if (sims_result != NULL) *sims_result << std::endl;
  return min_sim;
}
