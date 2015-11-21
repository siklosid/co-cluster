#include "AlgoBase.h"

#include "Common/Environment.h"

AlgoBase::AlgoBase(const CoClusterParams &params)
  : iter_finished_(0),
  num_iter_(params.num_iter),
  reader_control_(params, &data_set_),
  do_bicluster_(params.do_bicluster),
  row_view_(true),
//  need_last_iter_(false),
  last_iter_(false),
  similarity_factory_(new SimilarityFactory(params)),
  act_num_threads_(params.num_threads){

  // Create GobalInfo
  global_info_ = Environment::get().global_info();

  // Read the dataset
  reader_control_.StartRead();

  // Create computing threads
  par_comps_.resize(params.num_threads, NULL);
  for ( vector< ParallelComp* >::iterator it = par_comps_.begin();
        it != par_comps_.end(); it++ ) {
    *it = new ParallelComp(this);
  }

  // Create similarities
  SimilarityBase::global_info_ = global_info_;
  sims_.resize(params.num_datas, NULL);
  for (uint32_t i = 0; i < params.num_datas; i++) {
    sims_[i] = similarity_factory_->
               CreateSimilarity(string(params.similarity[i]).c_str(), data_set_[i], (do_bicluster_&&i==0)?true:false);
  }
}


AlgoBase::~AlgoBase() {
  log_status("Ending AlgoBase");
  for (uint32_t i = 0; i < Environment::get().params().num_threads; i++) {
    delete par_comps_[i];
  }

  for ( uint32_t i = 0; i < Environment::get().params().num_datas; i++ ) {
    delete sims_[i];
  }

  for (vector< Data<double>* >::iterator it = data_set_.begin(); it != data_set_.end(); it++) {
    delete *it;
  }

  delete global_info_;
  delete similarity_factory_;
}


void AlgoBase::Start() {
  timer.begin();

  log_info("Initializing algorithm");
  Init();

  while (iter_finished_ < num_iter_ && NeedMoreIter()) {
    row_view_ = true;
    MainCicle();
    if (do_bicluster_ && NeedMoreColIter()) {
      log_info("Row clustering finished");
      row_view_ = false;
      MainCicle();
    }

    iter_finished_++;
    log_info("Iter %d. finished", iter_finished_)
  }

  if (Environment::get().params().need_last_iter) {
    log_info("Last iter for doing some after processing.");
    row_view_ = true;
    last_iter_ = true;
    for (vector< SimilarityBase* >::iterator it = sims_.begin();
        it != sims_.end(); it++) {
      (*it)->SetLastIter(true);
    }

    MainCicle();

    for (vector< SimilarityBase* >::iterator it = sims_.begin();
      it != sims_.end(); it++) {
      (*it)->SetLastIter(false);
    }
    log_info("Last iter finished.");
  }

  log_info("Finishing algorithm")
  Finish();
  log_info_t("Algorithm took %f secs", timer.secs_elapsed());
}


void AlgoBase::AddNodeToCluster(const uint32_t &node_id, const uint32_t &cluster_id, double *weight) {
  log_dbg("Adding node: %u to cluster: %u", node_id, cluster_id);
  if (row_view_) {
    for (vector< SimilarityBase* >::iterator it = sims_.begin();
        it != sims_.end(); it++) {
      if (weight != NULL) {
        (*it)->AddNodeToCluster(node_id, cluster_id, weight);
      } else {
        (*it)->AddNodeToCluster(node_id, cluster_id);
      }
    }
  } else {
    if (weight != NULL) {
      sims_[0]->AddNodeToCluster(node_id, cluster_id, weight);
    } else {
      sims_[0]->AddNodeToCluster(node_id, cluster_id);
    }
  }
  global_info_->AddNodeToCluster(node_id, cluster_id);
}


void AlgoBase::SetNumThreads(const int &num_threads) {
  log_status("Setting number of threads to %d", num_threads);
  if (num_threads < act_num_threads_) {
    for (int i = act_num_threads_ - 1; i > num_threads - 1 && i >= 0; i--) {
      par_comps_[i]->Pause();
    }
  } else if (num_threads > act_num_threads_) {
    for (int i = act_num_threads_; i <= num_threads - 1 && i < static_cast<int>(par_comps_.size()); i++) {
      par_comps_[i]->Resume();
    }
  }
  act_num_threads_ = num_threads;
}


void AlgoBase::ResumeThreads() {
  vector< ParallelComp* >::iterator it;
  for (it = par_comps_.begin(); it != par_comps_.end(); it++ ) {
    (*it)->Resume();
  }
  act_num_threads_ = par_comps_.size();
}

void AlgoBase::Iter() {
  while ( running_iter_ ) {
    log_dbg("Still running");
    //sleep(1);
  }
}


void AlgoBase::MainCicle() {
  InitIter();


  vector< ParallelComp* >::iterator it;
  int i = 0;
  for ( it = par_comps_.begin(); it != par_comps_.end(); it++, i++) {
    (*it)->Start();
    if (i >= act_num_threads_) {
      (*it)->Pause();
    }
  }

  // Iter();

  for ( it = par_comps_.begin(); it != par_comps_.end(); it++ ) {
    (*it)->WaitForThread();
    if (act_num_threads_ < static_cast<int>(par_comps_.size())) {
      vector< ParallelComp* >::iterator it;
      for (it = par_comps_.begin(); it != par_comps_.end(); it++ ) {
        (*it)->Resume();
      }
    }
  }

  FinishIter();

  //iter_finished_++;
  //log_info("Iter %d. finished", iter_finished_)
}


bool AlgoBase::NeedMoreIter() {

  return true;
}


bool AlgoBase::NeedMoreColIter() {

  return true;
}


ParallelComp::ParallelComp(AlgoBase *algo)
  : _algo(algo) {

}


void ParallelComp::Main() {
  if (_algo->ParallelStep()) {
    log_dbg("Another step made");
  } else {
    Stop();
  }
}
