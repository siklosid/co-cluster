template<class T>
Data<T>::Data()
    : num_rows_(0), num_cols_(0),
      row_view_(true) {}


template<class T>
Data<T>::Data(const uint32_t &num_rows, const uint32_t &num_cols)
    : num_rows_(num_rows), num_cols_(num_cols),
      row_view_(true) {}


template<class T>
Data<T>::~Data() {}


template<class T>
uint32_t Data<T>::GetNumRows() {
  if (row_view_) {
    return num_rows_;
  } else {
    return num_cols_;
  }
}


template<class T>
uint32_t Data<T>::GetNumCols() {
  if (row_view_) {
    return num_cols_;
  } else {
    return num_rows_;
  }
}


template<class T>
void Data<T>::SetRowView(const bool &row_view) {
  row_view_ = row_view;
}
