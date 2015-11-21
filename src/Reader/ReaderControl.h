/*! ReaderControl controls the Reader thread objects that reads the */
/*! different parts of the dataset. In Constructor it gets */
/*! the type of the readers, than creates them by the help of */
/*! ReaderFactory. It allows as much Reader threads to read as much */
/*! is given in parameter. */

#ifndef __READER_CONTROL_H
#define __READER_CONTROL_H

#include <vector>
#include <string>

#include "Utils/log.h"
#include "Utils/Mutex.h"
#include "Utils/CoClusterParams.h"

#include "ReaderFactory.h"
#include "ReaderBase.h"

using std::vector;
using std::string;


class ReaderControl {

 public:
  ReaderControl(const CoClusterParams &params,
		vector< Data<double>* > *data_set);
  ~ReaderControl();

  /*! Calls the Start function of all reader thread in readers_ vect */
  void StartRead();

  /*! Reader object calls this function in its main cycle before starts reading.  */
  /*! If the number of readers who are reading at the moment reaches the limit  */
  /*! then it stops on mutex_read_ mutex */
  void CanIRead();

  /*! Reader object calls this function in its main cycle after reading.  */
  /*! If the number of readers who are reading at the moment is just went */
  /*! under the limit then it releases the mutex_read_ mutex */
  void FinishedRead();

  /*! Reads test data into the same dataset continuously */
  void StartReadTest();

 private:
  /*! Gets the readers string from params and returns a vector of the reader types */
  void GetReaderTypes(const string &readers, vector< string > *reder_types_vect);

  /*! Gets the input_files string from params and returns a vector of the input_files */
  void GetInputFiles(const string &input_files, vector< string > *input_files_vect);


  /*! Each element of the vector points to a partition of the dataset. */
  DataSet *data_set_;
  /*! For every part of the dataset it stores a reader */
  vector< ReaderBase* > readers_;

  /*! Counts the number of threads who are reading at the moment */
  int semafor_;
  /*! Number of threads */
  int num_threads_;

  Mutex mutex_semafor_;
  Mutex mutex_read_;

  ReaderFactory *reader_factory_;
};

#endif // __READER_CONTROL_H
