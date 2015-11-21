#include "ReaderControl.h"

#include "Common/Environment.h"
#include "Common/DenseData.h"
#include "Common/SparseData.h"

ReaderControl::ReaderControl(const CoClusterParams &params,
			     vector< Data<double>* > *data_set) :
    data_set_(data_set),
    semafor_(0),
    num_threads_(params.num_reader_threads) {

  log_dbg("ReaderFactory starts");
  reader_factory_ = new ReaderFactory(params);

  // Init vector of readers and datas
  readers_.resize(params.num_datas, NULL);
  data_set_->resize(params.num_datas, NULL);


  log_dbg("Creating readers");
  for (uint32_t i = 0; i < params.num_datas; i++) {
    if (string(params.data_type[i]) == "dense") {
      (*data_set)[i] = new DenseData<double>();
    } else {
      (*data_set)[i] = new SparseData<double>();
    }
    readers_[i] =  reader_factory_->CreateReader(string(params.input_dir).append(string(params.input_file[i])),
						 string(params.reader[i]),
						 (*data_set_)[i], this);
    log_status("Reader %d created", i);
  }
}


ReaderControl::~ReaderControl() {
  for (vector< ReaderBase* >::iterator it = readers_.begin();
       it != readers_.end(); it++) {
    delete *it;
  }
  delete reader_factory_;
}


void ReaderControl::StartRead() {
  for (vector< ReaderBase* >::iterator it = readers_.begin();
       it != readers_.end(); it++) {
    (*it)->Start();
  }

  for (vector< ReaderBase* >::iterator it = readers_.begin();
       it != readers_.end(); it++) {
    (*it)->WaitForThread();
  }

}


void ReaderControl::CanIRead() {

  mutex_read_.Lock();
  log_fdbg("CRmutex_read locked");
  mutex_semafor_.Lock();
  log_fdbg("CRmutexsemafor_ locked");
  semafor_++;
  mutex_semafor_.Unlock();
  log_fdbg("CRmutexsemafor_ unlocked");
  log_fdbg("CRsemafor: %d, num_threads: %d", semafor_, num_threads_);
  if ( semafor_ < num_threads_ ) {
    mutex_read_.Unlock();
    log_fdbg("CRmutex_read unlocked");
  }
}


void ReaderControl::FinishedRead() {

  mutex_semafor_.Lock();
  log_fdbg("FRmutexsemafor_ locked");
  log_fdbg("FRsemafor: %d, num_threads: %d", semafor_, num_threads_);
  if ( semafor_ == num_threads_ ) {
    mutex_read_.Unlock();
    log_fdbg("FRmutex_read unlocked");
  }
  semafor_--;
  mutex_semafor_.Unlock();
  log_fdbg("FRmutexsemafor_ unlocked");
}


void ReaderControl::StartReadTest() {
  // Setting readers to the test file
  for (uint32_t i = 0; i < Environment::get().params().num_datas; ++i) {
    delete readers_[i];
    readers_[i] = reader_factory_->CreateReader(string(Environment::get().params().input_dir).append(string(Environment::get().params().input_test_file[i])),
                                        string(Environment::get().params().reader[i]),
                                        (*data_set_)[i], this);
  }

  // Start reading the test files
  for (vector< ReaderBase* >::iterator it = readers_.begin();
       it != readers_.end(); it++) {
    (*it)->Start();
  }

  for (vector< ReaderBase* >::iterator it = readers_.begin();
       it != readers_.end(); it++) {
    (*it)->WaitForThread();
  }
}

void ReaderControl::GetReaderTypes(const string &readers,
				   vector< string > *reader_types_vect) {

  size_t start_pos = 0;
  size_t end_pos = 0;
  int num = 0;

  while ((end_pos = string(readers).find("|", start_pos)) != string::npos) {
    string reader_type = string(readers).substr(start_pos,
						end_pos - start_pos);
    reader_types_vect->push_back(reader_type);

    start_pos = end_pos++;
    num++;
  }

  string reader_type =
    string(readers).substr(start_pos, string(readers).length());
  reader_types_vect->push_back(reader_type);
}


void ReaderControl::GetInputFiles(const string &input_files,
				  vector< string > *input_files_vect) {

  size_t start_pos = 0;
  size_t end_pos = 0;
  int num = 0;

  while ((end_pos = string(input_files).find("|", start_pos)) != string::npos) {
    string input_file = string(input_files).substr(start_pos,
						   end_pos - start_pos);
    input_files_vect->push_back(input_file);

    start_pos = end_pos++;
    num++;
  }

  string input_file =
    string(input_files).substr(start_pos, string(input_files).length());
  input_files_vect->push_back(input_file);
}
