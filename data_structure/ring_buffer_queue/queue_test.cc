#include "data_structure/ring_buffer_queue/queue.h"

#include "gtest/gtest.h"

namespace cpputil {
namespace data_structure {

TEST(QueueTest, usage) {
  Queue queue(5);

  for (uint32_t i = 0; i < 10; ++i) {
    queue.Enqueue(i);
  }

  std::cout << "Dequeued item: " << queue.Dequeue() << std::endl;

  queue.Enqueue(10);
  queue.Enqueue(11);
  queue.Enqueue(12);

  while (!queue.IsEmpty()) {
    std::cout << "Dequeued item: " << queue.Dequeue() << std::endl;
  }
}

}  // namespace data_structure
}  // namespace cpputil
