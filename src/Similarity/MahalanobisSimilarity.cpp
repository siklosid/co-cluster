#include "MahalanobisSimilarity.h"

#include <math.h>

#include <fstream>


MahalanobisSimilarity::~MahalanobisSimilarity() {
  log_status("Ending Mahalanobis similarity");
  delete local_info_;
}


void MahalanobisSimilarity::Init() {
  local_info_ = new LocalInfo<L2ItemInfo, L2ClusterInfo, L2CoClusterInfo>();
  local_info_->Init(NULL, // row_item_info
                    NULL, // col_item_info
                    new vector<L2ClusterInfo>(global_info_->GetNumRowClusts(),
                        L2ClusterInfo(data_->GetNumCols())), // row_clust_info
                    (do_bicluster_?
                     new vector<L2ClusterInfo>(global_info_->GetNumColClusts(),
                         L2ClusterInfo(global_info_->GetNumRowItems())):
                     NULL), // col_clust_info
                    NULL); // co_cluster_info

  // First column is an indicator for training/test data
  uint32_t num_cols = data_->GetNumCols();

  double cov_matrix[num_cols*num_cols];
  for (uint32_t i = 0; i < num_cols*num_cols; i++) {
    cov_matrix[i] = 0;
  }
  cov_matrix_ = new double [num_cols*num_cols];
  vector<double> avarages(num_cols, 0.0);
  uint32_t num_train = 0;

  // Computing avarages
  for (uint32_t node_id = 0; node_id < data_->GetNumRows(); ++node_id) {
    Data<double>::const_iterator_base *it = data_->begin(node_id);
    Data<double>::const_iterator_base *end = data_->end(node_id);

    if (!global_info_->IsTrain(node_id)) continue;
    num_train++;
    for (; *it != *end; ++(*it)) {
      avarages[it->GetID()] += **it;
    }
  }
  for (uint32_t i = 0; i < avarages.size(); ++i) {
    avarages[i] /= num_train;
    log_dbg("Avg[%d]=%f", i, avarages[i]);
  }

  // Computing the covariance matrix
  for (uint32_t node_id = 0; node_id < data_->GetNumRows(); ++node_id) {
    Data<double>::const_iterator_base *it = data_->begin(node_id);
    Data<double>::const_iterator_base *it2;
    Data<double>::const_iterator_base *end = data_->end(node_id);

    uint32_t col_id;
    for (; *it != *end; ++(*it)) {
      col_id = it->GetID();
      it2 = data_->begin(node_id);
      for (; *it2 != *end; ++(*it2)) {
        uint32_t col2_id = it2->GetID();
        cov_matrix[col_id*num_cols + col2_id] +=
          (**it - avarages[col_id])*(**it2 - avarages[col2_id])/num_train;
      }
    }
  }

  // Inverting the covariance matrix
  InvertMatrix(cov_matrix, cov_matrix_, num_cols);
}


void MahalanobisSimilarity::InitIter(bool row_view) {

  log_dbg("InitIter: %s", row_view?"true":"false");
  real_iter_ = false;
  local_info_->SetRowView(row_view);
  data_->SetRowView(row_view);
  tmp_clust_info_ =
    new vector<L2ClusterInfo>(global_info_->GetNumRowClusts(),
                              L2ClusterInfo(data_->GetNumCols()));

  /*for (uint32_t clust_id = 0; clust_id < global_info_->GetNumRowClusts(); clust_id++) {
    if (local_info_->GetRowClusterInfo(clust_id).num_elements_ == 0) continue;
    (*tmp_clust_info_)[clust_id].num_elements_++;
    uint32_t num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_;
    for (uint32_t col_id = 0; col_id < global_info_->GetNumColItems(); col_id++) {
      (*tmp_clust_info_)[clust_id].col_sum_[col_id] +=
      local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id]/num_elements;
    }
  }*/
}


double MahalanobisSimilarity::SimilarityToCluster(const uint32_t &node_id,
    const uint32_t &clust_id) {

  log_dbg("Measuring the similarity of node: %u to cluster: %u", node_id, clust_id);
  double sim = 0.0;

  uint32_t num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_;
  if (num_elements == 0) {
    num_elements = 1;
    log_warn("Cluster is empty");
    //return 0.0;
  }

  uint32_t num_cols = data_->GetNumCols();

  double avgs[num_cols];
  for (uint32_t i = 0; i < num_cols; ++i) {
    avgs[i] = (local_info_->GetRowClusterInfo(clust_id).col_sum_[i])/num_elements;
    log_dbg("XXXavg[%d] = %f (%f %u)", i, avgs[i], local_info_->GetRowClusterInfo(clust_id).col_sum_[i], num_elements);
  }

  for (uint32_t i = 0; i < num_cols; ++i) {
    Data<double>::const_iterator_base *it = data_->begin(node_id);
    Data<double>::const_iterator_base *end = data_->end(node_id);

    uint32_t col_id;
    double s = 0.0;
    for (; *it != *end; ++(*it)) {
      col_id = it->GetID();
      log_dbg("XXX %f: %f", **it - avgs[col_id], cov_matrix_[num_cols*col_id + i]);
      s += (**it - avgs[col_id])*cov_matrix_[num_cols*col_id + i];
    }
    log_dbg("XXXs[%d]: %f", i, s);

    sim += s*(data_->GetItem(node_id, i) - avgs[i]);


    delete it;
    delete end;
  }
  sim = sqrt(sim);

  log_dbg("Similarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim);
  return sim;
}


double MahalanobisSimilarity::VectorSimilarityToCluster(const vector<double> &data_vector,
    const uint32_t &clust_id) {

  double sim = 0.0;

  uint32_t num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_;
  if (num_elements == 0) {
    num_elements = 1;
    log_warn("Cluster is empty");
    return 0.0;
  }

//  Data<double>::const_iterator_base *it = data_->begin(node_id);
//  Data<double>::const_iterator_base *end = data_->end(node_id);



  for (uint32_t col_id = 0; col_id < data_vector.size(); ++col_id) {
    double col_sum_avg =
      local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id]/num_elements;
    // sim += pow(data_.GetItem(node_id, col_id) - col_sum_avg, 2);
    double diff = data_vector[col_id] - col_sum_avg;
    log_fdbg("Item: %f, col_sum_avg: %f, Diff: %f", data_vector[col_id], col_sum_avg, diff);
    sim += diff*diff;
  }
//  delete it;
//  delete end;

  sim = sqrt(sim);

  log_dbg("Similarity to clust %d is: %f", clust_id, sim);
  return sim;
}


double MahalanobisSimilarity::SimilarityToBiCluster(const uint32_t &node_id,
    const uint32_t &clust_id) {

  log_dbg("Measuring the similarity of node: %u to cluster: %u", node_id, clust_id);
  double sim = 0.0;

  uint32_t num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_;
  if (num_elements == 0) {
    num_elements = 0;
    log_warn("Cluster is empty");
    return 0.0;
  }
  // This vector stores the similarities of the column clusters
  vector<double> sims(global_info_->GetNumColClusts(), 0.0);

  Data<double>::const_iterator_base *it = data_->begin(node_id);
  Data<double>::const_iterator_base *end = data_->end(node_id);
  uint32_t col_id;
  for (; *it != *end; ++(*it)) {
    col_id = it->GetID();
    // If column is not yet clustered we simply step to the next column
    if (global_info_->GetColClust(col_id) == numeric_limits<uint32_t>::max()) {
      continue;
    }
    double col_sum_avg =
      local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id]/num_elements;
    // sim += pow(data_.GetItem(node_id, col_id) - col_sum_avg, 2);
    double diff = **it - col_sum_avg;
    log_fdbg("Item: %f, col_sum_avg: %f, Diff: %f", **it, col_sum_avg, diff);
    sims[global_info_->GetColClust(col_id)] += diff*diff;
  }
  delete it;
  delete end;
  for (uint32_t col_clust_id = 0; col_clust_id < sims.size(); ++col_clust_id) {
    sim += sqrt(sims[col_clust_id]);
  }
  sim /= global_info_->GetNumColClusts();
  log_dbg("Similarity of node: %u to cluster: %u is: %f", node_id, clust_id, sim);
  return sim;
}


void MahalanobisSimilarity::AddNodeToCluster(const uint32_t &node_id,
    const uint32_t &cluster_id) {

  if (!global_info_->IsTrain(node_id)) return;

  real_iter_ = true;
  log_dbg("Adding node: %u to cluster: %u", node_id, cluster_id);
  data_mutex_.Lock();
  (*tmp_clust_info_)[cluster_id].num_elements_++;

  Data<double>::const_iterator_base* it = data_->begin(node_id);
  Data<double>::const_iterator_base* end = data_->end(node_id);
  uint32_t col_id;
  for (; *it != *end; ++(*it)) {
    col_id = it->GetID();
    (*tmp_clust_info_)[cluster_id].col_sum_[col_id] += **it;
  }
  delete it;
  delete end;
  data_mutex_.Unlock();
}


void MahalanobisSimilarity::AddVectorToCluster(const vector<double> &avg, const uint32_t &cluster_id) {
  log_dbg("Adding vector to cluster: %u", cluster_id);
  (*tmp_clust_info_)[cluster_id].num_elements_++;
  for (uint32_t col_id = 0; col_id < data_->GetNumCols(); col_id++) {
    (*tmp_clust_info_)[cluster_id].col_sum_[col_id] += avg[col_id];
  }
}


void MahalanobisSimilarity::FinishIter() {
  if (real_iter_) {
    local_info_->SwapClusterInfo(tmp_clust_info_);
    //if (do_bicluster_) local_info_->SwapCoClusterInfo(tmp_co_clust_info_);
  } else {
    delete tmp_clust_info_;
    //if (do_bicluster_) delete tmp_co_clust_info_;
  }
}


void MahalanobisSimilarity::Finish() {
  if (Environment::get().params().do_output_clust_centers) {
    string output_dir = Environment::get().params().output_dir;
    std::ofstream clust_centers((output_dir + Environment::get().params().output_clust_centers).c_str());
    if (clust_centers.fail()) {
      log_err("Couldn't open file: %s", (output_dir + Environment::get().params().output_clust_centers).c_str());
    }

    log_status("Writing out the row cluster centers");
    local_info_->SetRowView(true);
    uint32_t num_clusts, num_cols;
    bool row_view = global_info_->GetRowView();
    if (row_view) {
      num_clusts = global_info_->GetNumRowClusts();
      num_cols = data_->GetNumCols();
    } else {
      num_clusts = global_info_->GetNumColClusts();
      num_cols = global_info_->GetNumRowItems();
    }
    for (uint32_t clust_id = 0; clust_id < num_clusts; clust_id++) {
      uint32_t num_elements = 0;
      if ((num_elements = local_info_->GetRowClusterInfo(clust_id).num_elements_) == 0) continue;
      for (uint32_t col_id = 0; col_id < num_cols; col_id++) {
        clust_centers << local_info_->GetRowClusterInfo(clust_id).col_sum_[col_id]/num_elements << " ";
      }
      clust_centers << std::endl;
    }
  }

//  if (!Environment::get().params().do_bicluster) return;
//  log_status("Writing out the col cluster centers");
//  for (uint32_t clust_id = 0; clust_id < global_info_->GetNumColClusts(); clust_id++) {
//    uint32_t num_elements = 0;
//    if ((num_elements = local_info_->GetColClusterInfo(clust_id).num_elements_) == 0) continue;
//    fprintf(stdout, "%u", clust_id);
//    for (uint32_t col_id = 0; col_id < global_info_->GetNumRowItems(); col_id++) {
//      //fprintf(stdout, " %f", local_info_->GetColClusterInfo(clust_id).col_sum_[col_id]/num_elements);
//    }
//    fprintf(stdout, "\n");
//  }
}


void MahalanobisSimilarity::InvertMatrix(double *Min, double *Mout, int actualsize) {
  /* This function calculates the inverse of a square matrix
   *
   * matrix_inverse(double *Min, double *Mout, int actualsize)
   *
   * Min : Pointer to Input square Double Matrix
   * Mout : Pointer to Output (empty) memory space with size of Min
   * actualsize : The number of rows/columns
   *
   * Notes:
   *  - the matrix must be invertible
   *  - there's no pivoting of rows or columns, hence,
   *        accuracy might not be adequate for your needs.
   *
   * Code is rewritten from c++ template code Mike Dinolfo
   */
  /* Loop variables */
  int i, j, k;
  /* Sum variables */
  double sum,x;

  /*  Copy the input matrix to output matrix */
  for(i=0; i<actualsize*actualsize; i++) {
    Mout[i]=Min[i];
    log_dbg("ConvM[%d,%d]=%f", i%actualsize, (i - i%actualsize)/actualsize, Min[i]);
  }

  /* Add small value to diagonal if diagonal is zero */
  for(i=0; i<actualsize; i++)
  {
    j=i*actualsize+i;
    if((Mout[j]<1e-12)&&(Mout[j]>-1e-12)) {
      Mout[j]=1e-12;
    }
  }

  /* Matrix size must be larger than one */
  if (actualsize <= 1) return;

  for (i=1; i < actualsize; i++) {
    Mout[i] /= Mout[0]; /* normalize row 0 */
  }

  for (i=1; i < actualsize; i++)  {
    for (j=i; j < actualsize; j++)  { /* do a column of L */
      sum = 0.0;
      for (k = 0; k < i; k++) {
        sum += Mout[j*actualsize+k] * Mout[k*actualsize+i];
      }
      Mout[j*actualsize+i] -= sum;
    }
    if (i == actualsize-1) continue;
    for (j=i+1; j < actualsize; j++)  {  /* do a row of U */
      sum = 0.0;
      for (k = 0; k < i; k++) {
        sum += Mout[i*actualsize+k]*Mout[k*actualsize+j];
      }
      Mout[i*actualsize+j] = (Mout[i*actualsize+j]-sum) / Mout[i*actualsize+i];
    }
  }
  for ( i = 0; i < actualsize; i++ ) { /* invert L */
    for ( j = i; j < actualsize; j++ )  {
      x = 1.0;
      if ( i != j ) {
        x = 0.0;
        for ( k = i; k < j; k++ ) {
          x -= Mout[j*actualsize+k]*Mout[k*actualsize+i];
        }
      }
      Mout[j*actualsize+i] = x / Mout[j*actualsize+j];
    }
  }
  for ( i = 0; i < actualsize; i++ ) { /* invert U */
    for ( j = i; j < actualsize; j++ )  {
      if ( i == j ) continue;
      sum = 0.0;
      for ( k = i; k < j; k++ ) {
        sum += Mout[k*actualsize+j]*( (i==k) ? 1.0 : Mout[i*actualsize+k] );
      }
      Mout[i*actualsize+j] = -sum;
    }
  }
  for ( i = 0; i < actualsize; i++ ) { /* final inversion */
    for ( j = 0; j < actualsize; j++ )  {
      sum = 0.0;
      for ( k = ((i>j)?i:j); k < actualsize; k++ ) {
        sum += ((j==k)?1.0:Mout[j*actualsize+k])*Mout[k*actualsize+i];
      }
      Mout[j*actualsize+i] = sum;
      log_dbg("InvertedM[%d, %d]=%f", i, j, sum );
    }
  }
}

