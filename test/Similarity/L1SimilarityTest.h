#include <gtest/gtest.h>
#include <math.h>

#include "Common/Environment.h"
#include "Common/Data.h"
#include "Reader/ReaderControl.h"

#include "Similarity/L1Similarity.h"


class L1SimilarityTest : public testing::Test {

public:
  L1SimilarityTest() {
    data_set_ = new vector<Data<double>*>;
    reader_control_ = new ReaderControl(Environment::get().params() , data_set_);
    global_info_ = Environment::get().global_info();
    // Read the dataset.

    reader_control_->StartRead();
    global_info_->Init(6, 6);
    SimilarityBase::global_info_ = global_info_;
    similarity_ = new L1Similarity((*data_set_)[0],
                                   Environment::get().params().do_bicluster);
    similarity_->Init();
    global_info_->InitIter(true);
    similarity_->InitIter(true);
    AddNodeToCluster(0, 0);
    AddNodeToCluster(1, 0);
    AddNodeToCluster(2, 1);
    AddNodeToCluster(3, 1);
    AddNodeToCluster(4, 2);
    AddNodeToCluster(5, 2);
    similarity_->FinishIter();
    global_info_->FinishIter();
    if (Environment::get().params().do_bicluster) {
      global_info_->InitIter(false);
      similarity_->InitIter(false);
      AddNodeToCluster(0, 0);
      AddNodeToCluster(1, 0);
      AddNodeToCluster(2, 0);
      AddNodeToCluster(3, 1);
      AddNodeToCluster(4, 1);
      AddNodeToCluster(5, 1);
      similarity_->FinishIter();
      global_info_->FinishIter();
    }
    // global_info_->InitIter(true);
    // similarity_->InitIter(true);
  }

  virtual void SetUp() {

  }

  ~L1SimilarityTest() {
    similarity_->Finish();
    global_info_->Finish();
    for (uint32_t i = 0; i < Environment::get().params().num_datas; i++) {
      delete (*data_set_)[i];
    }

    delete data_set_;
    delete reader_control_;
    delete similarity_;
    SimilarityBase::global_info_ = NULL;
  }

  void AddNodeToCluster(const uint32_t &node_id, const uint32_t &clust_id) {
    similarity_->AddNodeToCluster(node_id, clust_id);
    global_info_->AddNodeToCluster(node_id, clust_id);
  }

  vector< Data<double>* > *data_set_;
  ReaderControl *reader_control_;
  L1Similarity *similarity_;
  GlobalInfo* global_info_;
};


TEST_F(L1SimilarityTest, SimilarityToClusterCol) {
  if (!Environment::get().params().do_bicluster) return;
  ASSERT_TRUE(Environment::get().params().do_bicluster) <<
      "This test is SKIPPED due to do_bicluster is set to false\n";
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(0, 0), 0);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(1, 0), 0);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(2, 0), 0);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(3, 0), 18);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(0, 1), 18);

}


TEST_F(L1SimilarityTest, SimirityToBiClusterCol) {
  /*ASSERT_TRUE(Environment::get().params().do_bicluster) <<
      "This test is SKIPPED due to do_bicluster is set to false\n";*/
  if (!Environment::get().params().do_bicluster) return;
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToBiCluster(0, 0), 0);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToBiCluster(0, 1), 6);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToBiCluster(4, 0), 6);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToBiCluster(4, 1), 0);
}



TEST_F(L1SimilarityTest, SimilarityToCluster) {
  global_info_->InitIter(true);
  similarity_->InitIter(true);

  EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(0, 0), 3);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(1, 0), 3);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(2, 0), 15);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(3, 0), 21);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(0, 2), 39);
}


TEST_F(L1SimilarityTest, SimirityToBiCluster) {
  if (!Environment::get().params().do_bicluster) return;
  ASSERT_TRUE(Environment::get().params().do_bicluster) <<
      "This test is SKIPPED due to do_bicluster is set to false\n";
  global_info_->InitIter(true);
  similarity_->InitIter(true);

  EXPECT_DOUBLE_EQ(similarity_->SimilarityToBiCluster(0, 0), 3.0/2);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToBiCluster(3, 0), 21.0/2);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToBiCluster(1, 1), 15.0/2);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToBiCluster(5, 2), 3.0/2);
  EXPECT_DOUBLE_EQ(similarity_->SimilarityToBiCluster(5, 1), 21.0/2);

}

