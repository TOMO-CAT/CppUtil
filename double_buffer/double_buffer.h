#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace util {

template <typename T>
// #include <map>
// #include <string>
// using T = std::map<int, std::string>;
class DoubleBuffer {
 public:
  using UpdaterFunc = std::function<void(T* const write_buffer_data)>;

 public:
  explicit DoubleBuffer(const T& data) {
    buffers_[0] = std::make_shared<T>(data);
    buffers_[1] = std::make_shared<T>(data);
    read_idx_ = 0;
  }
  explicit DoubleBuffer(const T&& data) {
    buffers_[0] = std::make_shared<T>(std::move(data));
    buffers_[1] = std::make_shared<T>(*buffers_[0]);
    read_idx_ = 0;
  }

 public:
  /**
   * @brief Get read_buffer and you can read it without restriction
   *
   * @return std::shared_ptr<T>
   */
  std::shared_ptr<T> Load() const {
    return buffers_[read_idx_];
  }

  /**
   * @brief It will block until it update both two buffers successfully
   *        Only once writer thread can perform this function at the same time
   *
   */
  void Update(const UpdaterFunc& updater) {
    // use std::mutex to update exclusively in multiple writer thread scenarios
    std::lock_guard<std::mutex> lock(write_mtx_);

    // update both two buffers, sleep to avoid data race on std::shared_ptr
    std::shared_ptr<T> write_buffer = monopolizer_writer_buffer();
    updater(write_buffer.get());
    this->swap_buffer();
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    write_buffer = monopolizer_writer_buffer();
    updater(write_buffer.get());
  }

  /**
   * @brief Reset the value of write buffer
   *
   * @param data
   */
  void Reset(const T& data) {
    this->Update([&](T* const write_buffer_data) {
      // deep copy
      *write_buffer_data = data;
    });
  }

 private:
  // wait all reads task on this buffer to get write buffer
  std::shared_ptr<T> monopolizer_writer_buffer() {
    int write_idx = 1 - read_idx_;
    while (buffers_[write_idx].use_count() != 1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return buffers_[write_idx];
  }

  void swap_buffer() {
    read_idx_ = 1 - read_idx_;
  }

 private:
  std::shared_ptr<T> buffers_[2];
  std::atomic<int> read_idx_;
  std::mutex write_mtx_;
};

}  // namespace util
