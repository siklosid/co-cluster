/*! Data stores the corresponding partition of the dataset in a dense 2d array format. */
/*! The same functions can be used when we doing a row clustering as when we */
/*! doing a column clustering, so from outside we don't have to know that which */
/*! one are we doing. */
/*! Data has a const_iterator class which works just like an stl iterator except */
/*! that functions 'begin' and 'end' take an uint32_t argument which tells them which */
/*! row or column we would like to iterate. */
/*! Since Data is template we can use it for storing any kind of objects. */

#ifndef __DATA_H
#define __DATA_H


#include <inttypes.h>

#include <vector>

#include "Utils/log.h"


using std::vector;
template<class T>
class Data {
public:
  // Iterator class
  class const_iterator_base {
    public:
    //const_iterator_base(const_iterator_base &other) {}
    //const_iterator_base() {}

    virtual T& operator*() = 0;
    virtual T* operator->() = 0;
    virtual void operator++() = 0;
    virtual bool operator!=(const_iterator_base& other) = 0;
    virtual uint32_t GetID() = 0;
  };

public:
  /*! Constructor when we don't know the size of the dataset */
  Data();
  /*! Constructor when we know the size of the dataset */
  Data(const uint32_t &num_rows, const uint32_t &num_cols);
  virtual ~Data();

  /*! Returns the number of rows of the data */
  uint32_t GetNumRows();

  /*! Returns the number of colums of the data */
  uint32_t GetNumCols();

  /*! Returns the appropriate element of the data */
  virtual T& GetItem(const uint32_t &row_id, const uint32_t &col_id) = 0;

  /*! Sets the appropriate element of the data. If we don't know the real size */
  /*! of the data and col_id or row_id is bigger than max, then it expands the */
  /*! size of matrix_. */
  virtual void SetItem(const uint32_t &row_id, const uint32_t &col_id, const T &val) = 0;

  /*! Sets the view point of class Data */
  void SetRowView(const bool &row_view);

  /*! Returns a const_iterator that points to the begining of row/col: row_id */
  virtual const_iterator_base* begin(uint32_t row_id) = 0;

  /*! Returns a const_iterator that points to the end of row/col: row_id */
  virtual const_iterator_base* end(uint32_t row_id) = 0;


protected:

  /*! The actual number of rows */
  uint32_t num_rows_;
  /*! The actual number of columns */
  uint32_t num_cols_;

  /*! If true then view point is from rows. If it is false then view point */
  /*! is from columns */
  bool row_view_;

};

typedef vector< Data<double>* > DataSet;

#include "Data.tcc"


#endif //  __DATA_H

