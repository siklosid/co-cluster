/* SimpleReader reads a raw matrix without any meta informations. The format of */
/* the input file is the following: */
/*   - one line of the matrix is one line in the file */
/*   - the elements of the line is seperated by space or tab */


#ifndef __SIMPLE_READER_H
#define __SIMPLE_READER_H

#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>

#include "Utils/log.h"
#include "ReaderBase.h"


//using namespace std;


class SimpleReader : public ReaderBase {

 public:
  SimpleReader(uint32_t num_lines, string input_file,
	       Data<double>* data, ReaderControl *reader_control);
  void Read(uint32_t num_lines);


 private:
  std::ifstream input_stream_;
  uint32_t num_lines_;
};

#endif // __SIMPLE_READER_H
