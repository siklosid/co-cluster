/*! Testing SimpleReader. Tests if the readed data is equivalent with the original */
/*! data in the test files. */

#include <gtest/gtest.h>

#include "Common/Environment.h"
#include "Common/Data.h"
#include "Reader/ReaderControl.h"

#include "Utils/log.h"


class ReaderTest : public testing::Test {

public:
  ReaderTest() {
    data_set_ = new vector<Data<double>*>;
    reader_control_ = new ReaderControl(Environment::get().params() , data_set_);
    log_status("Creating dataset");

    reader_control_->StartRead();
    (*data_set_)[0]->SetRowView(true);
  }

  ~ReaderTest() {
    for (uint32_t i = 0; i < Environment::get().params().num_datas; i++) {
      delete (*data_set_)[i];
    }
    delete data_set_;
    delete reader_control_;
  }

  vector< Data<double>* > *data_set_;
  ReaderControl *reader_control_;
};


//! Tests the first data
TEST_F(ReaderTest, DenseDataSet1) {
  for (int i = 0; i <= 5; i++) {
    for (int j = 0; j <= 2; j++) {
      EXPECT_EQ(i*2+1, (*data_set_)[0]->GetItem(i, j)) << "i: " << i << "j: " << j;
    }
    for (int j = 3; j <= 5; j++) {
      EXPECT_EQ( ((i%2 == 1)?i:(i+1)), (*data_set_)[0]->GetItem(i, j)) << "i: " << i << "j: " << j;
    }
  }
}

//! Tests the second data
TEST_F(ReaderTest, DenseDataSet2) {
  if (Environment::get().params().num_datas < 2) return;
  for (int i = 0; i <= 5; i++) {
    for (int j = 0; j <= 2; j++) {
      EXPECT_EQ((*data_set_)[1]->GetItem(i, j) , 12 - (i*2+1));
    }
    for (int j = 3; j <= 5; j++) {
      EXPECT_EQ((*data_set_)[1]->GetItem(i, j) , 6 - ((i%2 == 1)?i:(i+1)));
    }
  }
}

//! Tests the third data (sparse)
TEST_F(ReaderTest, SparseDataSet) {
  if (Environment::get().params().num_datas < 3) return;

  EXPECT_EQ(1,(*data_set_)[2]->GetItem(0, 0));
  EXPECT_EQ(4,(*data_set_)[2]->GetItem(0, 3));
  EXPECT_EQ(3,(*data_set_)[2]->GetItem(0, 5));
  EXPECT_EQ(1,(*data_set_)[2]->GetItem(2, 4));
  EXPECT_EQ(3,(*data_set_)[2]->GetItem(4, 5));
  EXPECT_EQ(3,(*data_set_)[2]->GetItem(5, 1));
  EXPECT_EQ(8,(*data_set_)[2]->GetItem(5, 5));
}
