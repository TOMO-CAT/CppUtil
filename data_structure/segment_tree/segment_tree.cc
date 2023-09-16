#include "data_structure/segment_tree/segment_tree.h"

#include "logger/log.h"

namespace cpputil {
namespace data_structure {

SegmentTree::SegmentTree(const std::size_t length) : root_(1), root_range_(1, length) {
  CHECK_GT(length, 0);
  CHECK_EQ((length & (length - 1)), 0);
  sum_.resize(length << 1);
  lazy_add_.resize(length << 1);
  Build(root_, root_range_);
}

void SegmentTree::Build(const std::size_t current, const Range& current_range) {
  // 到达叶子节点: 存储数组值
  if (current_range.length() == 1) {
    sum_[current] = lazy_add_[current_range.left];
    return;
  }

  // 递归构造线段树
  const std::size_t middle = current_range.middle();
  Build(current << 1, Range(current_range.left, middle));
  Build(current << 1 | 1, Range(middle + 1, current_range.right));

  // 从底到上更新统计信息
  PushUp(current);
}

int SegmentTree::Query(const Range& query_range, const std::size_t current, const Range& current_range) {
  // current_range 完全在 query_range 内直接返回
  if (current_range.left >= query_range.left && current_range.right <= query_range.right) {
    return sum_[current];
  }

  const std::size_t middle = current_range.middle();
  PushDown(current, current_range);

  int res = 0;
  if (query_range.left <= middle) {
    res += Query(query_range, current << 1, Range(current_range.left, middle));
  }
  if (query_range.right >= middle + 1) {
    res += Query(query_range, current << 1 | 1, Range(middle + 1, current_range.right));
  }

  return res;
}

void SegmentTree::Update(const std::size_t k, const int32_t increment, const std::size_t current,
                         const Range& current_range) {
  // 到达叶子节点, 修改
  if (current_range.length() == 1) {
    sum_[current] += increment;
    return;
  }

  // 判断在左子树还是右子树, 递归修改
  const std::size_t middle = current_range.middle();
  if (k <= middle) {
    Update(k, increment, current << 1, Range(current_range.left, middle));
  } else {
    Update(k, increment, current << 1 | 1, Range(middle + 1, current_range.right));
  }

  // 由于子节点更新了, 因此本节点也需要更新
  PushUp(current);
}

void SegmentTree::Update(const Range& k_range, const int32_t increment, const std::size_t current,
                         const Range& current_range) {
  // 如果 current_range 完全在 k_range 内
  if (k_range.left <= current_range.left && k_range.right >= current_range.right) {
    sum_[current] += (increment * current_range.length());
    // 增加懒惰标记, 表示本区间的 sum 正确, 但是子区间的 sum 仍然需要根据 lazy_add_ 的值来调整
    lazy_add_[current] += increment;
    return;
  }
  const std::size_t middle = current_range.middle();
  PushDown(current, current_range);
  if (k_range.left <= middle) {
    Update(k_range, increment, current << 1, Range(current_range.left, middle));
  }
  if (k_range.right >= middle + 1) {
    Update(k_range, increment, current << 1 | 1, Range(middle + 1, current_range.right));
  }

  PushUp(current);
}

void SegmentTree::PushDown(const std::size_t current, const Range& current_range) {
  if (lazy_add_[current] == 0) {
    return;
  }

  const std::size_t left = current << 1;
  const std::size_t right = current << 1 | 1;

  Range left_range(current_range.left, current_range.middle());
  Range right_range(current_range.middle() + 1, current_range.right);

  lazy_add_[left] += lazy_add_[current];
  lazy_add_[right] += lazy_add_[current];
  sum_[left] += (lazy_add_[current] * left_range.length());
  sum_[right] += (lazy_add_[current] * right_range.length());

  lazy_add_[current] = 0;
}

void SegmentTree::PushUp(const std::size_t current) {
  sum_[current] = sum_[current << 1] + sum_[current << 1 | 1];
}

}  // namespace data_structure
}  // namespace cpputil
