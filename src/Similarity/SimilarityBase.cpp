#include "SimilarityBase.h"

const GlobalInfo* SimilarityBase::global_info_ = NULL;

SimilarityBase::SimilarityBase(Data<double> *data, bool do_bicluster)
  : data_(data), do_bicluster_(do_bicluster),
    real_iter_(false), last_iter_(false) {
}


void SimilarityBase::SetLastIter(bool last_iter) {
  last_iter_ = last_iter;
}
