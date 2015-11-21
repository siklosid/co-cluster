/* Factory class for Similarity objects */

#ifndef __SIMILARITY_FACTORY_H
#define __SIMILARITY_FACTORY_H

#include <string>

#include "Utils/CoClusterParams.h"
#include "Common/Data.h"

class SimilarityBase;
class AlgoBase;

using std::string;


class SimilarityFactory {

 public:
  SimilarityFactory(const CoClusterParams &params);

  /* Creates the reader corresponding to the given reader_type */
  SimilarityBase* CreateSimilarity(const string &similarity_type,
				   Data<double> *data, bool do_bilcuster);

 private:
  const CoClusterParams &params_;
  bool do_biclustering_;
};

#endif // __SIMILARITY_FACTORY_H
