template<class T>
SparseData<T>::SparseData()
    : Data<T>() {

  columns_.clear();
  rows_.clear();
}


template<class T>
SparseData<T>::SparseData(const uint32_t &num_rows, const uint32_t &num_cols)
    : Data<T>(num_rows, num_cols) {

  columns_.clear();
  rows_.clear();
}


template<class T>
SparseData<T>::~SparseData() {
  // log_status("Destroying SparseData");

}


template<class T>
T& SparseData<T>::GetItem(const uint32_t &row_id, const uint32_t &col_id) {
  if (Data<T>::row_view_) {
    vector<Item*>::iterator it = rows_[row_id].begin();
    while(col_id != (*it)->col_id_ && it != rows_[row_id].end()) {
      ++it;
    }
    return (*it)->item_;
  } else {
    vector<Item*>::iterator it = columns_[row_id].begin();
    while(col_id != (*it)->col_id_) {
      ++it;
    }
    return (*it)->item_;
  }
}


template<class T>
void SparseData<T>::SetItem(const uint32_t &row_id, const uint32_t &col_id, const T &val) {
  typename map< uint32_t, vector< Item*> >::iterator it;
  Item* item = new Item(row_id, col_id, val);
  if ((it = rows_.find(row_id)) != rows_.end()) {
    it->second.push_back(item);
  } else {
    if (row_id + 1> Data<T>::num_rows_) Data<T>::num_rows_ = row_id + 1;
    rows_[row_id].clear();
    rows_[row_id].push_back(item);
  }


  if ((it = columns_.find(col_id)) != columns_.end()) {
    it->second.push_back(item);
  } else {
    if (col_id + 1 > Data<T>::num_cols_) Data<T>::num_cols_ = col_id + 1;
    columns_[col_id].clear();
    columns_[col_id].push_back(item);
  }

}

