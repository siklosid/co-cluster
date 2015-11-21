/*! Testing iterators of Data */

#include <gtest/gtest.h>

#include "Common/Environment.h"
#include "Common/Data.h"
#include "Reader/ReaderControl.h"

#include "Utils/log.h"


class DataTest : public testing::Test {

public:
  DataTest() {
    data_set_ = new vector<Data<double>*>;
    reader_control_ = new ReaderControl(Environment::get().params() , data_set_);
    log_status("Creating dataset");

    reader_control_->StartRead();
  }

  ~DataTest() {
    for (uint32_t i = 0; i < Environment::get().params().num_datas; i++) {
      delete (*data_set_)[i];
    }
    delete data_set_;
    delete reader_control_;
  }

  vector< Data<double>* > *data_set_;
  ReaderControl *reader_control_;
};

TEST_F(DataTest, DenseRowIterator) {
  double data[] = {3.0, 3.0, 3.0, 1.0, 1.0, 1.0};

  (*data_set_)[0]->SetRowView(true);
  Data<double>::const_iterator_base* it = (*data_set_)[0]->begin(1);
  Data<double>::const_iterator_base* end = (*data_set_)[0]->end(1);
  for (;*it != *end; ++(*it)) {
    EXPECT_EQ(data[it->GetID()], **it);
  }
}


TEST_F(DataTest, DenseColIterator) {
  double data[] = {1.0, 3.0, 5.0, 7.0, 9.0, 11.0};

  (*data_set_)[0]->SetRowView(false);
  Data<double>::const_iterator_base* it = (*data_set_)[0]->begin(0);
  for (;*it != *((*data_set_)[0]->end(0)); ++(*it)) {
    EXPECT_EQ(data[it->GetID()], **it);
  }
}


TEST_F(DataTest, SparseRowIterator) {
  double data[] = {0, 3.0, 0, 0, 0, 8.0};

  (*data_set_)[1]->SetRowView(true);
  Data<double>::const_iterator_base* it = (*data_set_)[1]->begin(5);
  Data<double>::const_iterator_base* end = (*data_set_)[1]->end(5);
  for (;*it != *end; ++(*it)) {
    EXPECT_EQ(data[it->GetID()], **it);
  }
}


TEST_F(DataTest, SparseColIterator) {
  double data[] = {3.0, 0, 0, 0, 3.0, 8.0};

  (*data_set_)[1]->SetRowView(false);
  Data<double>::const_iterator_base* it = (*data_set_)[1]->begin(5);
  Data<double>::const_iterator_base* end = (*data_set_)[1]->end(5);
  for (;*it != *end; ++(*it)) {
    EXPECT_EQ(data[it->GetID()], **it);
  }
}

TEST_F(DataTest, FastSparseRowIterator) {
  double data[] = {0, 3.0, 0, 0, 0, 8.0};

  (*data_set_)[2]->SetRowView(true);
  Data<double>::const_iterator_base* it = (*data_set_)[2]->begin(5);
  Data<double>::const_iterator_base* end = (*data_set_)[2]->end(5);
  for (;*it != *end; ++(*it)) {
    EXPECT_EQ(data[it->GetID()], **it);
  }
}


TEST_F(DataTest, FastSparseColIterator) {
  double data[] = {3.0, 0, 0, 0, 3.0, 8.0};

  (*data_set_)[2]->SetRowView(false);
  Data<double>::const_iterator_base* it = (*data_set_)[2]->begin(5);
  Data<double>::const_iterator_base* end = (*data_set_)[2]->end(5);
  for (;*it != *end; ++(*it)) {
    EXPECT_EQ(data[it->GetID()], **it);
  }
}

