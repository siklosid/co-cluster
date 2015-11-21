/*! SparseReader reads a raw matrix without any meta informations. The format of */
/*! the input file is the following: */
/*!   - one line of the file is one element of the matrix: */
/*!      row_id col_id value */
/*!   - the elements of the line is seperated by space or tab */
/*!   - it has to be sorted */


#ifndef __SPARSE_READER_H
#define __SPARSE_READER_H

#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>

#include "Utils/log.h"
#include "ReaderBase.h"



class SparseReader : public ReaderBase {

 public:
  SparseReader(uint32_t num_lines, string input_file,
	       Data<double>* data, ReaderControl *reader_control);
  void Read(uint32_t num_lines);


 private:
  std::ifstream input_stream_;
  uint32_t num_lines_;
};

#endif // __SPARSE_READER_H
