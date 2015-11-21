#include "ReaderBase.h"


ReaderBase::ReaderBase(uint32_t num_lines, Data<double>* data,
		       ReaderControl *reader_control) :
    has_more_lines_(true),
    data_(data),
    reader_control_(reader_control),
    num_lines_(num_lines) {

}


void ReaderBase::Main() {
  log_dbg("start reading");
  if (has_more_lines_) {
    log_fdbg("Asking for reading")
    reader_control_->CanIRead();
    Read(num_lines_);
    log_fdbg("Signs that finished reading");
    reader_control_->FinishedRead();
  } else {
    Stop();
  }
}
