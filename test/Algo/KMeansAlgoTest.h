#include <gtest/gtest.h>

#include "Common/Environment.h"
#include "Algo/KMeansAlgo.h"


class KMeansAlgoTest : public testing::Test {
  struct Result {
    Result(vector<uint32_t> *v1, vector<uint32_t> *v2) {
      row_item_to_cluster = *v1;
      col_item_to_cluster = *v2;
    }

    vector<uint32_t> row_item_to_cluster;
    vector<uint32_t> col_item_to_cluster;
  };

public:
  KMeansAlgoTest() {
    results = new vector<Result>;
    results->clear();
    kmeans = new KMeansAlgo(Environment::get().params());

    // KMeansAlgo init function but with fixed inital clustering
    kmeans->global_info_->Init(kmeans->data_set_[0]->GetNumRows(), kmeans->data_set_[0]->GetNumCols());
    for (vector< SimilarityBase* >::iterator it = kmeans->sims_.begin(); it != kmeans->sims_.end(); it++) {
      (*it)->Init();
    }

    kmeans->global_info_->InitIter(kmeans->row_view_);
    kmeans->row_moved_items_ = kmeans->global_info_->GetNumRowItems();
    for (vector< SimilarityBase* >::iterator it = kmeans->sims_.begin(); it != kmeans->sims_.end(); it++) {
      (*it)->InitIter(kmeans->row_view_);
    }

    // Init row clusters
    kmeans->AddNodeToCluster(0, 0);
    kmeans->AddNodeToCluster(1, 1);
    kmeans->AddNodeToCluster(4, 2);


    for (vector< SimilarityBase* >::iterator it = kmeans->sims_.begin(); it != kmeans->sims_.end(); it++) {
      (*it)->FinishIter();
    }
    kmeans->global_info_->FinishIter();

    if (kmeans->do_bicluster_) {
      kmeans->row_view_ = false;
      kmeans->global_info_->InitIter(kmeans->row_view_);
      kmeans->col_moved_items_ = kmeans->global_info_->GetNumRowItems();
      for (vector< SimilarityBase* >::iterator it = kmeans->sims_.begin(); it != kmeans->sims_.end(); it++) {
        (*it)->InitIter(kmeans->row_view_);
      }

      // Init column clusters
      kmeans->AddNodeToCluster(0, 0);
      kmeans->AddNodeToCluster(3, 1);


      for (vector< SimilarityBase* >::iterator it = kmeans->sims_.begin(); it != kmeans->sims_.end(); it++) {
        (*it)->FinishIter();
      }
      kmeans->global_info_->FinishIter();
    }

    while (kmeans->iter_finished_ < kmeans->num_iter_ && kmeans->NeedMoreIter()) {
      kmeans->row_view_ = true;
      kmeans->MainCicle();
      if (kmeans->do_bicluster_) {
        kmeans->row_view_ = false;
        kmeans->MainCicle();
      }

      kmeans->iter_finished_++;
      results->push_back(Result(Environment::get().global_info()->GetRowItemToCluster(),
                                Environment::get().global_info()->GetColItemToCluster()));
      log_info("Iter %d. finished", kmeans->iter_finished_);
    }

    log_info("Finishing algorithm");
    kmeans->Finish();
  }

  ~KMeansAlgoTest() {
    delete kmeans;
  }

  KMeansAlgo* kmeans;
  vector<Result> *results;
};


TEST_F(KMeansAlgoTest, proba_test) {

  // iter 1 row
  EXPECT_EQ((*results)[0].row_item_to_cluster[0], 0);
  EXPECT_EQ((*results)[0].row_item_to_cluster[1], 1);
  EXPECT_EQ((*results)[0].row_item_to_cluster[2], 1);
  EXPECT_EQ((*results)[0].row_item_to_cluster[3], 2);
  EXPECT_EQ((*results)[0].row_item_to_cluster[4], 2);
  EXPECT_EQ((*results)[0].row_item_to_cluster[5], 2);
  // iter 1 col
  EXPECT_EQ((*results)[0].col_item_to_cluster[0], 0);
  EXPECT_EQ((*results)[0].col_item_to_cluster[1], 0);
  EXPECT_EQ((*results)[0].col_item_to_cluster[2], 0);
  EXPECT_EQ((*results)[0].col_item_to_cluster[3], 1);
  EXPECT_EQ((*results)[0].col_item_to_cluster[4], 1);
  EXPECT_EQ((*results)[0].col_item_to_cluster[5], 1);

  // iter 2 row
  EXPECT_EQ((*results)[1].row_item_to_cluster[0], 0);
  EXPECT_EQ((*results)[1].row_item_to_cluster[1], 0);
  EXPECT_EQ((*results)[1].row_item_to_cluster[2], 1);
  EXPECT_EQ((*results)[1].row_item_to_cluster[3], 2);
  EXPECT_EQ((*results)[1].row_item_to_cluster[4], 2);
  EXPECT_EQ((*results)[1].row_item_to_cluster[5], 2);
  // Iter 2 col
  EXPECT_EQ((*results)[1].col_item_to_cluster[0], 0);
  EXPECT_EQ((*results)[1].col_item_to_cluster[1], 0);
  EXPECT_EQ((*results)[1].col_item_to_cluster[2], 0);
  EXPECT_EQ((*results)[1].col_item_to_cluster[3], 1);
  EXPECT_EQ((*results)[1].col_item_to_cluster[4], 1);
  EXPECT_EQ((*results)[1].col_item_to_cluster[5], 1);

/*  // iter 3 row
  EXPECT_EQ((*results)[2].row_item_to_cluster[0], 0);
  EXPECT_EQ((*results)[2].row_item_to_cluster[1], 0);
  EXPECT_EQ((*results)[2].row_item_to_cluster[2], 1);
  EXPECT_EQ((*results)[2].row_item_to_cluster[3], 1);
  EXPECT_EQ((*results)[2].row_item_to_cluster[4], 2);
  EXPECT_EQ((*results)[2].row_item_to_cluster[5], 2);
  // Iter 3 col
  EXPECT_EQ((*results)[2].col_item_to_cluster[0], 0);
  EXPECT_EQ((*results)[2].col_item_to_cluster[1], 0);
  EXPECT_EQ((*results)[2].col_item_to_cluster[2], 0);
  EXPECT_EQ((*results)[2].col_item_to_cluster[3], 1);
  EXPECT_EQ((*results)[2].col_item_to_cluster[4], 1);
  EXPECT_EQ((*results)[2].col_item_to_cluster[5], 1);
*/
}
