#include "ReaderFactory.h"
#include "SimpleReader.h"
#include "SparseReader.h"


ReaderFactory::ReaderFactory(const CoClusterParams &params) :
    params_(params) {

}



ReaderBase* ReaderFactory::CreateReader(const string &input_file,
					const string &reader_type,
					Data<double>* data,
					ReaderControl *reader_control) {

  log_info("Creating reader for file: %s", input_file.c_str());

  if (reader_type == "simple") {
    SimpleReader* reader = new SimpleReader(params_.num_lines, input_file, data, reader_control);
    log_dbg("Type of created reader is: simple");
    return reader;
  } else if (reader_type == "sparse") {
    SparseReader* reader = new SparseReader(params_.num_lines, input_file, data, reader_control);
    log_dbg("Type of created reader is: sparse");
    return reader;
  } else {
    log_err("Couldn't recognise reader type: %s", reader_type.c_str());
    return NULL;
  }
}
