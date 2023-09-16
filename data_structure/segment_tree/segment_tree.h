#pragma once

#include <cstdint>
#include <vector>

namespace cpputil {
namespace data_structure {

/**
 * @brief 区间 [left, right]
 *
 */
struct Range {
  explicit Range(const std::size_t l, const std::size_t r) : left(l), right(r) {
  }

  std::size_t length() const {
    return right - left + 1;
  }

  std::size_t middle() const {
    return ((left + right) >> 1);
  }

  std::size_t left = 0;
  std::size_t right = 0;
};

/**
 * @brief 线段树
 *
 */
class SegmentTree {
 public:
  explicit SegmentTree(const std::size_t length);

 public:
  /**
   * @brief 修改点, 例如将第 k 个元素的值加上 increment
   *
   * @param k
   * @param increment
   */
  void Update(const std::size_t k, const int32_t increment) {
    return Update(k, increment, root_, root_range_);
  }

  /**
   * @brief 区间修改, 例如将一个子区间 k_range 的值都加上 increment
   *
   */
  void Update(const Range& k_range, const int32_t increment) {
    return Update(k_range, increment, root_, root_range_);
  }

  /**
   * @brief 区间查询
   *
   * @param query_range
   * @return int
   */
  int Query(const Range& query_range) {
    return Query(query_range, root_, root_range_);
  }

 private:
  /**
   * @brief 对当前节点 current 表示的区间 current_range 建树
   *
   * @param current 当前节点下标, 根节点的下标为 1
   * @param current_range
   */
  void Build(const std::size_t current, const Range& current_range);

  /**
   * @brief 负责更新节点信息, 在这个场景中是求和
   *
   * @note current 节点的和直接等于左右子树和相加
   * @param current
   */
  void PushUp(const std::size_t current);

  /**
   * @brief 下推一层标记至左右子区间, 将当前区间 current_range 的 lazy_add_ 标记清零
   *
   * @param current 当前节点下标
   * @param current_range 当前区间
   */
  void PushDown(const std::size_t current, const Range& current_range);

  /**
   * @brief 点修改, 例如将第 k 个元素的值加上 increment
   *
   */
  void Update(const std::size_t k, const int32_t increment, const std::size_t current, const Range& current_range);

  /**
   * @brief 区间修改, 例如将一个子区间 k_range 的值都加上 increment
   *
   */
  void Update(const Range& k_range, const int32_t increment, const std::size_t current, const Range& current_range);

  /**
   * @brief 区间查询
   *
   * @param query_range
   * @param current
   * @param current_range
   * @return int
   */
  int Query(const Range& query_range, const std::size_t current, const Range& current_range);

 private:
  std::vector<int> sum_;       // 每个节点对应的区间和
  std::vector<int> lazy_add_;  // 懒惰 add 标记
  const std::size_t root_;
  const Range root_range_;
};

}  // namespace data_structure
}  // namespace cpputil
