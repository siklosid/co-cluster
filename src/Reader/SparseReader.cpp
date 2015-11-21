#include "SparseReader.h"
#include <stdio.h>


SparseReader::SparseReader(uint32_t num_lines, string input_file,
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


void SparseReader::Read(uint32_t num_lines) {
  log_fdbg("Reading %u lines", num_lines);
  string line;
  std::stringstream ss;

  for (uint32_t i = 0; i < num_lines && input_stream_.good(); i++) {
    line.clear();
    getline(input_stream_, line);

    if (line.size() > 0) {
      uint32_t row_id;
      uint32_t col_id;
      double value;
      char *spc;
      char *c = &line[0];

      // Get the row_id
      spc = strpbrk(c, " \t");
      if (spc != NULL) {
        *spc = '\0';
        row_id = atoi(c);
        c = spc + 1;
      } else {
        log_warn("Couldn't read row_id at line: %u, skipping line", num_lines_);
        num_lines_++;
        continue;
      }

      // Get the col_id
      spc = strpbrk(c, " \t");
      if (spc != NULL) {
        *spc = '\0';
        col_id = atoi(c);
        c = spc + 1;
      } else {
        log_warn("Couldn't read col_id at line: %u, skipping line", num_lines_);
        num_lines_++;
        continue;
      }

      // Get the value
      value = atof(c);


      data_->SetItem(row_id, col_id, value);
    }
    num_lines_++;
    if (num_lines_%10000 == 0) {
        log_status("Read %u lines", num_lines_);
    }
  }

  if (input_stream_.eof()) {
    log_dbg("Reached the end of the file");
    has_more_lines_ = false;
    input_stream_.close();
  }
}
