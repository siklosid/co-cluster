/* Factory class for Reader objects */

#ifndef __READER_FACTORY_H
#define __READER_FACTORY_H

#include <string>

#include "Utils/CoClusterParams.h"
#include "Common/Data.h"
class ReaderBase;
class ReaderControl;

using std::string;


class ReaderFactory {

 public:
  ReaderFactory(const CoClusterParams &params);

  /* Creates the reader corresponding to the given reader_type */
  ReaderBase* CreateReader(const string &input_file,
			   const string &reader_type,
			   Data<double> *data,
			   ReaderControl *reader_control);

 private:
  const CoClusterParams &params_;
};

#endif // __READER_FACTORY_H
