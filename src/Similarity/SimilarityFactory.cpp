#include "SimilarityFactory.h"
#include "L1Similarity.h"
#include "L2Similarity.h"
#include "KLSimilarity.h"
#include "JSSimilarity.h"
#include "KLConstSimilarity.h"
#include "SoftKLSimilarity.h"
#include "BregmanSimilarity.h"
#include "KernelSimilarity.h"
#include "MahalanobisSimilarity.h"


SimilarityFactory::SimilarityFactory(const CoClusterParams &params)
  : params_(params) {

}


SimilarityBase* SimilarityFactory::CreateSimilarity(const string &similarity_type,
						    Data<double> *data, bool do_bicluster) {

  if (similarity_type == "l1") {
    return new L1Similarity(data, do_bicluster);
  } else if (similarity_type == "l2") {
    return new L2Similarity(data, do_bicluster);
  } else if (similarity_type == "kl") {
    return new KLSimilarity(data, do_bicluster);
  } else if (similarity_type == "js") {
    return new JSSimilarity(data, do_bicluster);
  } else if (similarity_type == "kl_const") {
    return new KLConstSimilarity(data, do_bicluster);
  } else if (similarity_type == "kl_soft") {
    return new SoftKLSimilarity(data, do_bicluster);
  } else if (similarity_type == "bregman") {
    return new BregmanSimilarity(data, do_bicluster);
  } else if (similarity_type == "kernel") {
    return new KernelSimilarity(data, do_bicluster);
  } else if (similarity_type == "mahalanobis") {
    return new MahalanobisSimilarity(data, do_bicluster);
  } else {
    log_err("Couldn't recognise similarity type: %s", similarity_type.c_str());
    return NULL;
  }
}
