#include "SimpleReader.h"
#include <stdio.h>

#include "Common/Environment.h"


SimpleReader::SimpleReader(uint32_t num_lines, string input_file,
                           Data<double>* data, ReaderControl *reader_control) :
  ReaderBase::ReaderBase(num_lines, data, reader_control),
  input_stream_(input_file.c_str()) {

  if (input_stream_.fail()) {
    log_err("Input file: %s not found", input_file.c_str());
    exit(1);
  }

  num_lines_ = 0;
  log_status("Initializing data");
}


void SimpleReader::Read(uint32_t num_lines) {
  log_fdbg("Reading %u lines", num_lines);
  string id;
  string line;
  std::stringstream ss;

  bool ignore_id = Environment::get().params().ignore_id;

  for (uint32_t i = 0; i < num_lines && input_stream_.good(); i++) {
    line.clear();
    getline(input_stream_, line);

    if (line.size() > 0) {
      double value;
      char *spc;
      char *c = &line[0];

      // Ignore id if we have to
      if (ignore_id) {
        spc = strpbrk(c, " \t");
        *spc='\0';
        c = spc + 1;
      }

      // Parse the rest of the line
      uint32_t num_items_ = 0;
      while ((spc = strpbrk(c, " \t")) != NULL) {
        *spc = '\0';
        value = atof(c);
        c = spc + 1;
        data_->SetItem(num_lines_, num_items_, value);
        num_items_++;
      }

      value = atof(c);
      data_->SetItem(num_lines_, num_items_, value);
    }
    num_lines_++;
  }

  if (input_stream_.eof()) {
    log_dbg("Reached the end of the file");
    has_more_lines_ = false;
    input_stream_.close();
  }
}
