#include "GlobalInfo.h"

#include <fstream>

#include "Common/Environment.h"


GlobalInfo::GlobalInfo()
  : row_view_(true) {
    row_item_to_cluster_ = NULL;
    col_item_to_cluster_ = NULL;
  }


GlobalInfo::~GlobalInfo() {
  if (row_item_to_cluster_ != NULL) delete row_item_to_cluster_;
  if (col_item_to_cluster_ != NULL) delete col_item_to_cluster_;
}


void GlobalInfo::Init(uint32_t num_row_items, uint32_t num_col_items) {
  num_row_items_ = num_row_items;
  num_col_items_ = num_col_items;

  num_row_clusts_ = Environment::get().params().num_row_cluster;
  num_col_clusts_ = Environment::get().params().num_col_cluster;


  row_item_to_cluster_ = new vector<uint32_t>(num_row_items,
                                              numeric_limits<uint32_t>::max());
  col_item_to_cluster_ = new vector<uint32_t>(num_col_items,
                                              numeric_limits<uint32_t>::max());

  string is_train_file_name = Environment::get().params().is_train_filename;
  if (is_train_file_name != "") {
    is_train_ = new vector<bool>(num_row_items_, false);
    std::ifstream is_train_file(is_train_file_name.c_str());
    if (is_train_file.fail()) {
      log_err("Couldn't open file: %s", is_train_file_name.c_str());
    }

    uint32_t id = 0;
    uint32_t lab;
    while (is_train_file.good()) {
      if (id >= num_row_items_) {
        log_err("There is more labels (%u) in file: %s than the number of items we have (%u).",
                id, is_train_file_name.c_str(), num_row_items_);
        break;
      }

      is_train_file >> lab;
      if (lab == 1) {
        (*is_train_)[id] = true;
      }
      id++;
    }
  } else {
    is_train_ = NULL;
  }
}


void GlobalInfo::InitIter(bool row_view) {
  row_view_ = row_view;
  real_iter_ = false;
  if (row_view_) {
    tmp_item_to_cluster_ = new vector<uint32_t>(num_row_items_,
                                                numeric_limits<uint32_t>::max());
  } else {
    tmp_item_to_cluster_ = new vector<uint32_t>(num_col_items_,
                                                numeric_limits<uint32_t>::max());
  }
}


void GlobalInfo::FinishIter() {
  if (real_iter_) {
    if (row_view_) {
      row_item_to_cluster_->clear();
      delete row_item_to_cluster_;
      row_item_to_cluster_ = NULL;
      swap(tmp_item_to_cluster_, row_item_to_cluster_);
    } else {
      col_item_to_cluster_->clear();
      delete col_item_to_cluster_;
      col_item_to_cluster_ = NULL;
      swap(tmp_item_to_cluster_, col_item_to_cluster_);
    }
  } else {
    delete tmp_item_to_cluster_;
  }
}


uint32_t GlobalInfo::GetRowClust(uint32_t row_id) const {
  if ( row_view_ ) {
    return (*row_item_to_cluster_)[row_id];
  } else {
    return (*col_item_to_cluster_)[row_id];
  }
}


uint32_t GlobalInfo::GetColClust(uint32_t col_id) const {
  if ( row_view_ ) {
    return (*col_item_to_cluster_)[col_id];
  } else {
    return (*row_item_to_cluster_)[col_id];
  }
}


uint32_t GlobalInfo::GetNumRowItems() const {
  if ( row_view_ ) {
    return num_row_items_;
  } else {
    return num_col_items_;
  }
}


uint32_t GlobalInfo::GetNumColItems() const {
  if ( row_view_ ) {
    return num_col_items_;
  } else {
    return num_row_items_;
  }
}


uint32_t GlobalInfo::GetNumRowClusts() const {
  if ( row_view_ ) {
    return num_row_clusts_;
  } else {
    return num_col_clusts_;
  }
}


uint32_t GlobalInfo::GetNumColClusts() const {
  if ( row_view_ ) {
    return num_col_clusts_;
  } else {
    return num_row_clusts_;
  }
}


void GlobalInfo::SetRowView(bool row_view) {
  row_view_ = row_view;
}


void GlobalInfo::AddNodeToCluster(const uint32_t &node_id,
				  const uint32_t &cluster_id) {

  real_iter_ = true;
  (*tmp_item_to_cluster_)[node_id] = cluster_id;
}
