#include "data_structure/segment_tree/segment_tree.h"

#include "gtest/gtest.h"

namespace cpputil {
namespace data_structure {

TEST(SegmentTreeTest, usage) {
  SegmentTree segment_tree(1024);

  // 初始化时区间总和为 0
  EXPECT_EQ(0, segment_tree.Query(Range(1, 1024)));

  // 给第 10 个元素加上 1234
  segment_tree.Update(10, 1234);
  EXPECT_EQ(1234, segment_tree.Query(Range(1, 1024)));
  EXPECT_EQ(1234, segment_tree.Query(Range(10, 10)));

  // 给区间 [8,12] 都加上 20
  segment_tree.Update(Range(8, 12), 20);
  EXPECT_EQ(1234 + 20 * 5, segment_tree.Query(Range(1, 1024)));
  EXPECT_EQ(20 + 1234, segment_tree.Query(Range(10, 10)));
}

}  // namespace data_structure
}  // namespace cpputil
