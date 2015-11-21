#ifndef __CLUSTER_INFO_COLLECTION
#define __CLUSTER_INFO_COLLECTION

#include <math.h>

#include <vector>
#include <utility>


using std::vector;
using std::pair;


// L2 distance
class L2ItemInfo {
 public:
  L2ItemInfo()
    : sum_(0), log_sum_(0) {}

  double sum_;
  double log_sum_;
};


class L2ClusterInfo {
 public:
  L2ClusterInfo(const uint32_t &num_cols)
    : num_elements_(0), col_sum_(num_cols, 0.0) {}

  ~L2ClusterInfo() {
    col_sum_.clear();
  }


  uint32_t num_elements_;
  vector< double > col_sum_;
};


class L2CoClusterInfo {
 public:
  L2CoClusterInfo() {}
};


// KL Distance

class KLItemInfo {
 public:
  KLItemInfo()
    : sum_(0.0), log_sum_(0.0) {}

  double sum_;
  double log_sum_;
};


class KLClusterInfo {
 public:
  KLClusterInfo(const uint32_t &col_num)
    : num_elements_(0), log_sum_(0), col_sum_(col_num, 0.0) {}

  void LogAvg() {
    for (vector<double>::iterator it = col_sum_.begin(); it != col_sum_.end(); ++it) {
      if (*it != 0) *it = log(*it/num_elements_);
    }
  }

  uint32_t num_elements_;
  double log_sum_;
  vector< double > col_sum_;
};


class KLCoClusterInfo {
 public:
  KLCoClusterInfo()
    : log_sum_(0.0) {}

  void SetSum(const double &sum) { log_sum_ = sum; }
  double log_sum_;
};


// KL Distance with constraints.
enum State {
  TRUE = 0,
  FALSE = 1,
  NEUTRAL = 2
};


class KLConstItemInfo {
 public:
  KLConstItemInfo()
    : sum_(0.0), log_sum_(0.0), state_(NEUTRAL) {}

  double sum_;
  double log_sum_;
  State state_;
};


class KLConstClusterInfo {
 public:
  KLConstClusterInfo(const uint32_t &col_num)
    : num_elements_(0), log_sum_(0), col_sum_(col_num, 0.0), state_(NEUTRAL) {}

  void LogAvg() {
    for (vector<double>::iterator it = col_sum_.begin(); it != col_sum_.end(); ++it) {
      if (*it != 0) *it = log(*it/num_elements_);
    }
  }

  uint32_t num_elements_;
  double log_sum_;
  vector< double > col_sum_;
  State state_;
};


// KL Distance whenjelölt soft clustering
class SoftKLItemInfo {
  public:
  SoftKLItemInfo(const uint32_t &num_row_clusts)
      : sum_(0.0), log_sum_(0.0) {
    weights_.resize(num_row_clusts, 0.0);
  }

  double sum_;
  double log_sum_;
  vector<double> weights_;
};


// Jensen-Shannon Distance
class JSItemInfo {
 public:
  JSItemInfo()
    : sum_(0.0), log_sum_(0.0) {}

  double sum_;
  double log_sum_;
};



class JSClusterInfo {
 public:
  JSClusterInfo(const uint32_t &col_num)
    : num_elements_(0), sum_(0.0), col_sum_(col_num, 0.0)/*, log_sum_(0.0), col_sum_(col_num, 0.0)*/ {}

//  void LogAvg() {
//    for (vector<double>::iterator it = col_sum_.begin(); it != col_sum_.end(); ++it) {
//      if (*it != 0) *it = log(*it/num_elements_);
//    }
//  }

  void SetPreSim(const double &pre_sim) {
    pre_sim_ = pre_sim;
  }

  uint32_t num_elements_;
  double sum_;
  //double log_sum_;
  vector< double > col_sum_;
  double pre_sim_;
};


class JSCoClusterInfo {
 public:
  JSCoClusterInfo()
    : sum_(0.0), log_sum_(0.0) {}

  //void SetSum(const double &sum) { log_sum_ = sum; }
  double sum_;
  double log_sum_;
};


// Bregman distance
class BregmanItemInfo {
 public:

  uint32_t num_;
  double sum_;
  double avg_sum_;
};

class BregmanClusterInfo {
 public:
  BregmanClusterInfo()
    : num_(0), avg_sum_(0) {}

  uint32_t num_;
  double avg_sum_;
};

class BregmanCoClusterInfo {
 public:
  BregmanCoClusterInfo()
    : num_(0), avg_sum_(0) {}

  uint32_t num_;
  double avg_sum_;
};


// Kernel distance
class KernelItemInfo {};

class KernelClusterInfo {
  public:
    KernelClusterInfo()
      : num_elements_(0) {

      medoid_vals_.clear();
    }

    vector< pair<uint32_t, double> > medoid_vals_;
    uint32_t act_medoid_;
    uint32_t min_medoid_;
    uint32_t num_elements_;
};

class KernelCoClusterInfo {};


#endif // __CLUSTER_INFO_COLLECTION
