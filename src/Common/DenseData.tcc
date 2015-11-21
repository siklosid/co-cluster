template<class T>
DenseData<T>::DenseData()
    : Data<T>(),
      max_num_rows_(7), max_num_cols_(7) {

  matrix_ = new T*[max_num_rows_];
  for (uint32_t i = 0; i < max_num_rows_; i++) {
    matrix_[i] = new T[max_num_cols_];
  }
}


template<class T>
DenseData<T>::DenseData(const uint32_t &num_rows, const uint32_t &num_cols)
    : Data<T>(num_rows, num_cols),
      max_num_rows_(Data<T>::num_rows_+1), max_num_cols_(Data<T>::num_cols_ + 1) {

  matrix_ = new T*[max_num_rows_];
  for (uint32_t i = 0; i < max_num_rows_; i++) {
    matrix_[i] = new T[max_num_cols_];
  }
}


template<class T>
DenseData<T>::DenseData(const uint32_t &num_rows, const uint32_t &num_cols, T default_value)
    : Data<T>(num_rows, num_cols),
      max_num_rows_(Data<T>::num_rows_+1), max_num_cols_(Data<T>::num_cols_ + 1) {

  matrix_ = new T*[max_num_rows_];
  for (uint32_t i = 0; i < max_num_rows_; ++i) {
    matrix_[i] = new T[max_num_cols_];
    for (uint32_t j = 0; j < max_num_cols_; ++j) {
      matrix_[i][j] = default_value;
    }
  }
}


template<class T>
DenseData<T>::~DenseData() {
  // log_status("Destroying data");

  for (uint32_t i = 0; i < max_num_rows_; i++) {
    delete [] matrix_[i];
  }
  delete [] matrix_;
}


template<class T>
T& DenseData<T>::GetItem(const uint32_t &row_id, const uint32_t &col_id) {
  if (Data<T>::row_view_)
    return matrix_[row_id][col_id];
  else
    return matrix_[col_id][row_id];
}


template<class T>
void DenseData<T>::SetItem(const uint32_t &row_id, const uint32_t &col_id, const T &val) {
  // Check if the matrix have enough rows for this action
  //log_fdbg("Setting matrix[%u][%u]=%lf", row_id, col_id, val);
  if (row_id + 1 > Data<T>::num_rows_) {
    Data<T>::num_rows_ = row_id + 1;
    if (Data<T>::num_rows_ + 1 > max_num_rows_) {
      T **matrix_tmp = new T*[2*max_num_rows_];
      uint32_t i = 0;
      for (; i < Data<T>::num_rows_ - 1; ++i) {
        matrix_tmp[i] = matrix_[i];
      }
      for (;i < 2*max_num_rows_; ++i) {
        matrix_tmp[i] = new T[max_num_cols_];
      }
      for (i = 0; i < max_num_rows_; ++i) {
        if (i >= Data<T>::num_rows_ - 1) delete [] matrix_[i];
        matrix_[i] = NULL;
      }
      delete [] matrix_;
      matrix_ = matrix_tmp;
      max_num_rows_ *= 2;

      log_fdbg("Resizing number of rows to: %u", max_num_rows_);
    }
  }

  if (col_id + 1 > Data<T>::num_cols_) {
    Data<T>::num_cols_ = col_id + 1;
    if (Data<T>::num_cols_ + 1 > max_num_cols_) {
      T **matrix_tmp = new T*[max_num_rows_];
      uint32_t i = 0;
      for (; i < Data<T>::num_rows_; ++i) {
        matrix_tmp[i] = new T[max_num_cols_*2];
        for (uint32_t j = 0; j < Data<T>::num_cols_ - 1; ++j)
          matrix_tmp[i][j] = matrix_[i][j];
      }
      for (;i < max_num_rows_; ++i) {
        matrix_tmp[i] = new T[2*max_num_cols_];
      }
      for (i = 0; i < max_num_rows_; ++i) {
        delete [] matrix_[i];
      }
      delete [] matrix_;
      matrix_ = matrix_tmp;
      matrix_tmp = NULL;
      max_num_cols_ *= 2;

      log_fdbg("Resizing number of rows to: %u", max_num_rows_);
    }
  }

  matrix_[row_id][col_id] = val;
}

