#include <map>

#include "double_buffer/double_buffer.h"
#include "gtest/gtest.h"

TEST(DoubleBufferTest, update_test) {
  std::map<int, int> data;
  util::DoubleBuffer<std::map<int, int>> db(std::move(data));

  db.Update([](std::map<int, int>* write_buffer_data) {
    (*write_buffer_data)[1] = 1;
  });

  {
    auto read_buffer = db.Load();
    ASSERT_EQ(read_buffer->size(), 1u);
  }

  db.Update([](std::map<int, int>* write_buffer_data) {
    (*write_buffer_data)[2] = 1;
  });

  {
    auto read_buffer = db.Load();
    ASSERT_EQ(read_buffer->size(), 2u);
  }
}
