/* GlobalInfo stores global informations about the clustering */

#ifndef __GLOBAL_INFO_H
#define __GLOBAL_INFO_H

#include <stdint.h>
#include <vector>
#include <limits>

using std::vector;
using std::numeric_limits;

class GlobalInfo {
 public:
  GlobalInfo();
  ~GlobalInfo();

  /*! Sets the values of num_row/col_items/clusts */
  /*! Initialize the item_to_cluster vectors before every iteration */
  void Init(uint32_t num_row_items, uint32_t num_col_items);

  /*! Initialize temporary vectors */
  void InitIter(bool row_view);

  /*! Swaps temporary and acutal vectors after every iteration */
  void FinishIter();

  /*! Doing the necessary finalizations */
  void Finish() {}

  /*! Returns the cluster of the given item */
  uint32_t GetRowClust(uint32_t row_id) const;
  uint32_t GetColClust(uint32_t col_id) const;

  /*! Returns the number of items */
  uint32_t GetNumRowItems() const;
  uint32_t GetNumColItems() const;

  /*! Returns the number of clusters */
  uint32_t GetNumRowClusts() const;
  uint32_t GetNumColClusts() const;

  /*! Returns the item to cluster vectors */
  const vector<uint32_t>* GetRowItemToCluster() const { return row_item_to_cluster_; }
  const vector<uint32_t>* GetColItemToCluster() const { return col_item_to_cluster_; }

  /*! Returns that an item belongs to the train or to the test set */
  bool IsTrain(const uint32_t node_id) const {
    if (is_train_ == NULL) {
      return true;
    } else {
      return (*is_train_)[node_id];
    }
  }

  /*! Sets row_view_ */
  void SetRowView(bool row_view);

  /*! Returns row_view_ */
  bool GetRowView() const { return row_view_; }

  void AddNodeToCluster(const uint32_t &node_id, const uint32_t &cluster_id);

 private:
  /*! Mapping of items to their clusters */
  vector<uint32_t> *row_item_to_cluster_;
  vector<uint32_t> *col_item_to_cluster_;
  vector<uint32_t> *tmp_item_to_cluster_;

  /*! Stores a boolean information about the elements */
  vector<bool> *is_train_;

  /*! Number of items and clusters */
  uint32_t num_row_items_;
  uint32_t num_col_items_;
  uint32_t num_row_clusts_;
  uint32_t num_col_clusts_;

  /*! If true then view point id from rows if false then */
  /*! view point is from columns */
  bool row_view_;
  bool real_iter_;
};


#endif // __GLOBAL_INFO_H
