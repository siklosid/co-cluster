/*! Testing SimpleReader. Tests if the readed data is equivalent with the original */
/*! data in the test files. */

#include <gtest/gtest.h>

#include "Common/Environment.h"
#include "Common/Data.h"
#include "Reader/ReaderControl.h"

#include "Utils/log.h"


class SimpleReaderTest : public testing::Test {

public:
  SimpleReaderTest() {
    data_set_ = new vector<Data<double>*>;
    reader_control_ = new ReaderControl(Environment::get().params() , data_set_);
    log_status("Creating dataset");

    reader_control_->StartRead();
    (*data_set_)[0]->SetRowView(true);
  }

  ~SimpleReaderTest() {
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
TEST_F(SimpleReaderTest, DataSet1) {
  for (int i = 0; i <= 5; i++) {
    for (int j = 0; j <= 2; j++) {
      EXPECT_EQ((*data_set_)[0]->GetItem(i, j) , i*2+1) << "i: " << i << "j: " << j;
    }
    for (int j = 3; j <= 5; j++) {
      EXPECT_EQ((*data_set_)[0]->GetItem(i, j) , ((i%2 == 1)?i:(i+1)));
    }
  }
}

//! Tests the second data
TEST_F(SimpleReaderTest, DataSet2) {
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
