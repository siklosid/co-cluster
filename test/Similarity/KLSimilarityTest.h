#include <gtest/gtest.h>
#include <math.h>

#include "Common/Environment.h"
#include "Reader/ReaderControl.h"

#include "Similarity/KLSimilarity.h"


class KLSimilarityTest : public testing::Test {

public:
  KLSimilarityTest() {
    data_set_ = new vector<Data<double>*>;
    reader_control_ = new ReaderControl(Environment::get().params() , data_set_);
    global_info_ = Environment::get().global_info();
    // Read the dataset.

    reader_control_->StartRead();
    global_info_->Init(6, 6);
    SimilarityBase::global_info_ = global_info_;
    similarity_ = new KLSimilarity((*data_set_)[0], Environment::get().params().do_bicluster);
    similarity_->Init();
    global_info_->InitIter(true);
    similarity_->InitIter(true);
    AddNodeToCluster(0, 2);
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
      AddNodeToCluster(2, 1);
      AddNodeToCluster(3, 0);
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

  ~KLSimilarityTest() {
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
  KLSimilarity *similarity_;
  GlobalInfo* global_info_;
};


TEST_F(KLSimilarityTest, SimilarityToCluster) {
  if (!Environment::get().params().do_bicluster) {
    global_info_->InitIter(true);
    similarity_->InitIter(true);

    EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(0, 0), 0);
    EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(1, 0), 0);
    EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(2, 0), 0);
    EXPECT_DOUBLE_EQ(similarity_->SimilarityToCluster(3, 0), 0);
    EXPECT_NEAR(similarity_->SimilarityToCluster(0, 2), 2*0.05*(log(0.05/0.043333333)) +
                0.05*log(0.05/0.03), 1e-6);
  }
}


TEST_F(KLSimilarityTest, SimirityToBiCluster) {
  if (Environment::get().params().do_bicluster) {
    global_info_->InitIter(true);
    similarity_->InitIter(true);

    EXPECT_NEAR(similarity_->SimilarityToBiCluster(0, 0), 0.05/0.15*(2*log(1.388888889)+log(3.571428571)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(0, 1), 0.05/0.15*(log(1.785714286) + 2*log(2.777777777778)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(0, 2), 0.05/0.15*(2*log(1.697530864)+log(2.619047619)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(1, 0), 0.05/0.15*(2*log(1.388888889)+log(3.571428571)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(1, 1), 0.05/0.15*(log(1.785714286) + 2*log(2.777777777778)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(1, 2), 0.05/0.15*(2*log(1.697530864)+log(2.619047619)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(2, 0), 0.05/0.15*(log(1.785714286)+2*log(2.777777778)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(2, 1), 0.05/0.15*(2*log(1.388888889)+log(3.571428571)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(2, 2), 0.05/0.15*(2*log(2.037037037)+log(2.182539683)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(3, 0), 0.05/0.15*(log(1.785714286)+2*log(2.777777778)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(3, 1), 0.05/0.15*(2*log(1.388888889)+log(3.571428571)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(3, 2), 0.05/0.15*(2*log(2.037037037)+log(2.182539683)), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(4, 0), 0.04/0.2*(2*log(0.833333333)+ log(1.071428571)
                + 2*log(1.666666667)), 1e-6);
  }
}


TEST_F(KLSimilarityTest, SimirityToBiClusterCol) {
  if (Environment::get().params().do_bicluster) {
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(0, 0), 0.05/0.18*(log(1.388888889)+log(1.6975300864)) +
                0.04/0.18*2*log(1.018518519), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(0, 1), 0.05/0.18*(log(2.777777778)+log(2.037037037)) +
                0.04/0.18*2*log(1.222222222), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(1, 0), 0.05/0.18*(log(1.388888889)+log(1.6975300864)) +
                0.04/0.18*2*log(1.018518519), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(1, 1), 0.05/0.18*(log(2.777777778)+log(2.037037037)) +
                0.04/0.18*2*log(1.222222222), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(2, 0), 0.05/0.14*(log(0.5*3.571428571)+log(2.182539683)) +
                0.04/0.14*log(1.30952381), 1e-5);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(2, 1), 0.05/0.14*(log(3.571428571)+log(2.619047619)) +
                0.04/0.14*log(1.5714228571), 1e-5);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(3, 0), 0.05/0.14*(2*log(3.571428571)) +
                0.04/0.14*log(1.30952381), 1e-6);
    EXPECT_NEAR(similarity_->SimilarityToBiCluster(3, 1), 0.05/0.14*(2*log(0.5*3.571428571)) +
                0.04/0.14*log(1.571428571), 1e-6);
  }
}
