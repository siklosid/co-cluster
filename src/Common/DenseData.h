/*! Data stores the corresponding partition of the dataset in a dense 2d array format. */
/*! The same functions can be used when we doing a row clustering as when we */
/*! doing a column clustering, so from outside we don't have to know that which */
/*! one are we doing. */
/*! Data has a const_iterator class which works just like an stl iterator except */
/*! that functions 'begin' and 'end' take an uint32_t argument which tells them which */
/*! row or column we would like to iterate. */
/*! Since Data is template we can use it for storing any kind of objects. */

#ifndef _DENSE_DATA_H
#define _DENSE_DATA_H


#include <inttypes.h>

#include <vector>

#include "Utils/log.h"
#include "Common/ClusterInfoCollection.h"
#include "Common/Data.h"


using std::vector;


template<class T>
class DenseData : public Data<T> {

public:
  // Iterator class
  class const_iterator : public Data<T>::const_iterator_base {
  public:
    /*! Copy constructor */
    const_iterator(const_iterator &other) {
      this->matrix_ = other.matrix_;
      this->ptr = other.ptr;
      this->row_view_ = other.row_view_;
      this->col_id_ = other.col_id_;
      this->act_id_ = other.act_id_;
    }

    const_iterator(T **matrix, bool row_view, T *item, uint32_t col_id = 0)
      : matrix_(matrix), ptr(item), row_view_(row_view), col_id_(col_id), act_id_(0) {}

    inline T& operator*() {
      return *ptr;
    }

    inline T* operator->() {
      return ptr;
    }

    void operator++() {
      if (row_view_) {
        ++ptr;
      } else {
        ++matrix_;
        ptr = *matrix_ + col_id_;
      }
      act_id_++;
    }

    bool operator!=(Data<double>::const_iterator_base& other) {
      return this->ptr != static_cast<DenseData<double>::const_iterator&>(other).ptr;
    }

    bool operator!=(Data<KLCoClusterInfo>::const_iterator_base& other) {
      return this->ptr != static_cast<DenseData<KLCoClusterInfo>::const_iterator&>(other).ptr;
    }

    bool operator!=(Data<JSCoClusterInfo>::const_iterator_base& other) {
      return this->ptr != static_cast<DenseData<JSCoClusterInfo>::const_iterator&>(other).ptr;
    }

    bool operator!=(Data<BregmanCoClusterInfo>::const_iterator_base& other) {
      return this->ptr != static_cast<DenseData<BregmanCoClusterInfo>::const_iterator&>(other).ptr;
    }

    bool operator==(const_iterator &other) const {
      return this->ptr == other.ptr;
    }

    uint32_t GetID() {
      return act_id_;
    }

  private:
    /*! Pointer to the dataset. We need this to iterate through columns */
    T **matrix_;
    /*! This pointer points to the actual item */
    T *ptr;
    /*! For increasement we have to know that are we in row view or in column view */
    bool row_view_;
    /*! We need this information for iterating through columns */
    uint32_t col_id_;
    uint32_t act_id_;
  };


public:
  /*! Constructor when we don't know the size of the dataset */
  DenseData();
  /*! Constructor when we know the size of the dataset */
  DenseData(const uint32_t &num_rows, const uint32_t &num_cols);
  DenseData(const uint32_t &num_rows, const uint32_t &num_cols, T default_value);
  ~DenseData();

  /*! Returns the appropriate element of the data */
  T& GetItem(const uint32_t &row_id, const uint32_t &col_id);

  /*! Sets the appropriate element of the data. If we don't know the real size */
  /*! of the data and col_id or row_id is bigger than max, then it expands the */
  /*! size of matrix_. */
  void SetItem(const uint32_t &row_id, const uint32_t &col_id, const T &val);


  /*! Returns a const_iterator that points to the begining of row/col: row_id */
  const_iterator* begin(uint32_t row_id) {
    if (Data<T>::row_view_) {
      T* ptr = matrix_[row_id];
      return new const_iterator(matrix_, Data<T>::row_view_, ptr, row_id);
    } else {
      T* ptr = matrix_[0] + row_id;
      return new const_iterator(matrix_, Data<T>::row_view_, ptr, row_id);
    }
  }

  /*! Returns a const_iterator that points to the end of row/col: row_id */
  const_iterator* end(uint32_t row_id) {
    if (Data<T>::row_view_) {
      T* ptr = matrix_[row_id] + Data<T>::num_cols_;
      return new const_iterator(matrix_, Data<T>::row_view_, ptr, row_id);
    } else {
      T* ptr = *(matrix_ + Data<T>::num_rows_) + row_id;
      return new const_iterator(matrix_, Data<T>::row_view_, ptr, row_id);
    }
  }


private:
  /*! The 2d array which stores the elements of the input matrix */
  T **matrix_;

  /*! The maximum number of rows. If we find more rows while reading the data */
  /*! then we have to allocate memory for more rows in matrix_ */
  uint32_t max_num_rows_;
  /*! The maximum number of columns. If we find more columns while reading the data */
  /*! then we have to allocate memory for more columns in matrix_ */
  uint32_t max_num_cols_;

};

// typedef vector< Data<double>* > DataSet;

#include "DenseData.tcc"


#endif //  _DENSE_DATA_H

