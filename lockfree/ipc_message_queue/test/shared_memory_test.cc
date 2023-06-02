#include "boost/filesystem.hpp"
#include "boost/filesystem/file_status.hpp"
#include "boost/interprocess/mapped_region.hpp"
#include "boost/interprocess/shared_memory_object.hpp"
#include "boost/interprocess/sync/file_lock.hpp"
#include "boost/interprocess/sync/sharable_lock.hpp"
// NOLINT
#include "boost/interprocess/detail/file_wrapper.hpp"
#include "gtest/gtest.h"
#include "logger/log.h"

// 检查新创建的共享内存 size 是否为 0
TEST(SharedMemoryTest, new_shared_memory_size) {
  const char kSharedMemoryName[] = "SharedMemoryTest.new_shared_memory_size";
  boost::interprocess::shared_memory_object shm(boost::interprocess::open_or_create, kSharedMemoryName,
                                                boost::interprocess::read_write);
  std::int64_t current_size_bytes = 0;
  shm.get_size(current_size_bytes);
  EXPECT_EQ(current_size_bytes, 0);
  boost::interprocess::shared_memory_object::remove(kSharedMemoryName);
}

// 检查已有的共享内存 size
TEST(SharedMemoryTest, allocated_memory_size) {
  const char kSharedMemoryName[] = "SharedMemoryTest.allocated_memory_size";
  const std::int64_t desired_size_bytes = 20 * 1024 * 1024;  // 20 MB

  boost::interprocess::shared_memory_object::remove(kSharedMemoryName);

  // 1. 创建并分配共享内存
  {
    try {
      boost::interprocess::shared_memory_object shm(boost::interprocess::create_only, kSharedMemoryName,
                                                    boost::interprocess::read_write);
      shm.truncate(desired_size_bytes);
    } catch (boost::interprocess::interprocess_exception& ex) {
      LOG_FATAL << "Failed to create shared memory: " << ex.what();
    }
  }

  // 2. 重新打开共享内存, 检查分配空间大小
  {
    boost::interprocess::shared_memory_object shm(boost::interprocess::open_or_create, kSharedMemoryName,
                                                  boost::interprocess::read_write);
    std::int64_t current_size_bytes = 0;
    shm.get_size(current_size_bytes);
    EXPECT_EQ(current_size_bytes, desired_size_bytes);
    LOG_INFO << "current size bytes: " << current_size_bytes;
  }

  // 3. 删除共享内存
  boost::interprocess::shared_memory_object::remove(kSharedMemoryName);
}

// 打开不存在的共享内存文件
TEST(SharedMemoryTest, open_not_exist_shared_memory) {
  const char kSharedMemoryName[] = "SharedMemoryTest.open_not_exist_shared_memory";
  bool is_exist = false;
  try {
    ::boost::interprocess::shared_memory_object shared_memory(::boost::interprocess::open_only, kSharedMemoryName,
                                                              ::boost::interprocess::read_only);
    is_exist = true;
  } catch (boost::interprocess::interprocess_exception& e) {
    EXPECT_EQ("No such file or directory", std::string(e.what()));
  }
  EXPECT_TRUE(!is_exist);
}

// 给非零的共享内存分配空间 (truncate)
TEST(SharedMemoryTest, truncated_exist_shared_memory) {
  const char kSharedMemoryName[] = "SharedMemoryTest.truncated_exist_shared_memory";
  boost::interprocess::shared_memory_object::remove(kSharedMemoryName);

  auto GetSharedMemorySize = [](const ::boost::interprocess::shared_memory_object& shm) -> int64_t {
    int64_t shm_size = 0;
    shm.get_size(shm_size);
    return shm_size;
  };

  ::boost::interprocess::shared_memory_object shared_memory(::boost::interprocess::open_or_create, kSharedMemoryName,
                                                            ::boost::interprocess::read_write);
  shared_memory.truncate(2048);
  EXPECT_EQ(2048, GetSharedMemorySize(shared_memory));

  // 1. truncate 更多内存
  shared_memory.truncate(4096);
  EXPECT_EQ(4096, GetSharedMemorySize(shared_memory));

  // 2. truncate 更少内存
  shared_memory.truncate(1024);
  EXPECT_EQ(1024, GetSharedMemorySize(shared_memory));
}

// 确保 ::boost::interprocess::file_lock 析构函数会释放锁
TEST(SharedMemoryTest, lock_twice) {
  std::string file_lock_path = "./SharedMemoryTest.lock_twice.lock";
  if (!::boost::filesystem::exists(file_lock_path)) {
    LOG_INFO << "Creat file lock [" << file_lock_path << "]";
    ::boost::interprocess::ipcdetail::file_wrapper file(::boost::interprocess::open_or_create, file_lock_path.c_str(),
                                                        ::boost::interprocess::read_write);
  }

  for (int i = 0; i < 3; ++i) {
    ::boost::interprocess::file_lock file_lock(file_lock_path.c_str());

    LOG_INFO << "Lock [" << file_lock_path << "] to creating or resize shared memory";
    file_lock.lock();
    LOG_INFO << "Lock [" << file_lock_path << "] successfully";
  }
}
