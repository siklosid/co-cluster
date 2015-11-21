/*! Interface for reader classes. A reader reads a partition of */
/*! the dataset to read it into the memory from a file. */

#ifndef __READER_BASE_H
#define __READER_BASE_H

#include <stdio.h>

#include "Utils/ThreadManager.h"
#include "Common/Data.h"
#include "ReaderControl.h"


class ReaderBase : public ThreadManager {

 public:
  ReaderBase(uint32_t num_lines, Data<double> *data, ReaderControl *reader_control);
  void Main();

  //virtual void init();
  virtual void Read(uint32_t num_lines) = 0;
  //virtual void finish();


 protected:
  bool has_more_lines_;
  Data<double> *data_;
  ReaderControl *reader_control_;
  uint32_t num_lines_;

 private:


};


#endif // __READER_BASE_H
