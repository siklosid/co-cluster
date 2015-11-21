/*! Data stores the corresponding partition of the dataset in a dense 2d array format. */
/*! The same functions can be used when we doing a row clustering as when we */
/*! doing a column clustering, so from outside we don't have to know that which */
/*! one are we doing. */
/*! Data has a const_iterator class which works just like an stl iterator except */
/*! that functions 'begin' and 'end' take an uint32_t argument which tells them which */
/*! row or column we would like to iterate. */
/*! Since Data is template we can use it for storing any kind of objects. */

#ifndef __SPARSEDATA_H
#define __SPARSEDATA_H


#include <inttypes.h>

#include <vector>
#include <map>
//#include <list>

#include "Utils/log.h"
#include "Common/ClusterInfoCollection.h"
#include "Common/Data.h"

using std::vector;
using std::map;
//using std::list;


class Item {
 public:
  Item() : row_id_(0), col_id_(0), item_(0.0) {}
  Item(const uint32_t &row_id, const uint32_t &col_id, const double &val) :
       row_id_(row_id), col_id_(col_id), item_(val) {}

  uint32_t row_id_;
  uint32_t col_id_;
  double item_;
};


template<class T>
class SparseData : public Data<T> {

public:

  // Iterator class
  class const_iterator : public Data<T>::const_iterator_base {
  public:
    /*! Copy constructor */
    const_iterator(const_iterator &other) {
      this->it_ = other.it_;
      this->row_view_ = other.row_view_;
      this->act_id_ = other.act_id_;
    }

    const_iterator()
      :  act_id_(0) {}

    const_iterator(bool &row_view, vector<Item*>::iterator it)
      :  act_id_(0), it_(it), row_view_(row_view){}

    inline T& operator*() {
      return (*it_)->item_;
    }

    inline T* operator->() {
      return &(*it_)->item_;
    }

    inline void operator++() {
      it_++;
    }

    inline bool operator!=(Data<double>::const_iterator_base &other) {
      return this->it_ != static_cast<SparseData<double>::const_iterator&>(other).it_;
    }

    inline bool operator!=(Data<KLCoClusterInfo>::const_iterator_base &other) {
      return this->it_ != static_cast<SparseData<KLCoClusterInfo>::const_iterator&>(other).it_;
    }


    inline bool operator==(const const_iterator &other) const {
      return this->it_ == other.it_;
    }

    uint32_t GetID() {
      if (row_view_) {
        return (*it_)->col_id_;
      } else {
        return (*it_)->row_id_;
      }
    }

  private:
    /*! The actual id */
    uint32_t act_id_;
    /*! This pointer points to the actual item */
    vector<Item*>::iterator it_;
    /*! For increasement we have to know that are we in row view or in column view */
    bool row_view_;
  };


public:
  /*! Constructor when we don't know the size of the SparseDataset */
  SparseData();
  /*! Constructor when we know the size of the SparseDataset */
  SparseData(const uint32_t &num_rows, const uint32_t &num_cols);
  ~SparseData();


  /*! Returns the appropriate element of the SparseData */
  T& GetItem(const uint32_t &row_id, const uint32_t &col_id);

  /*! Sets the appropriate element of the SparseData. If we don't know the real size */
  /*! of the SparseData and col_id or row_id is bigger than max, then it expands the */
  /*! size of matrix_. */
  void SetItem(const uint32_t &row_id, const uint32_t &col_id, const T &val);

  /*! Returns a const_iterator that points to the begining of row/col: row_id */
  const_iterator* begin(uint32_t row_id) {
    if (Data<T>::row_view_) {
      vector<Item*>::iterator it = rows_[row_id].begin();
      return new const_iterator(Data<T>::row_view_, it);
    } else {
      vector<Item*>::iterator it = columns_[row_id].begin();
      return new const_iterator(Data<T>::row_view_, it);
    }
  }

  /*! Returns a const_iterator that points to the end of row/col: row_id */
  inline const_iterator* end(uint32_t row_id) {
    if (Data<T>::row_view_) {
      vector<Item*>::iterator it = rows_[row_id].end();
      return new const_iterator(Data<T>::row_view_, it);
    } else {
      vector<Item*>::iterator it = columns_[row_id].end();
      return new const_iterator(Data<T>::row_view_, it);
    }
  }

private:

  /*! Stores the columns of the dataset */
  map< uint32_t, vector< Item* > > columns_;
  /*! Stores the rows of the dataset */
  map< uint32_t, vector< Item* > > rows_;

};

//typedef vector< SparseData<double>* > DataSet;

#include "SparseData.tcc"


#endif //  __SPARSEDATA_H

