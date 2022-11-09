#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

template <typename T>
class DoubleBuffer {
 public:
    using UpdaterFunc = std::function<void(T& data)>;

 public:
    DoubleBuffer(std::shared_ptr<T> read_buffer, std::shared_ptr<T> write_buffer) {
        buffers_[0] = read_buffer;
        buffers_[1] = write_buffer;
        read_idx_ = 0;
    }

    explicit DoubleBuffer(std::shared_ptr<T> data_sptr) {
        buffers_[0] = data_sptr;
        buffers_[1] = std::make_shared<T>(*data_sptr.get());
        read_idx_ = 0;
    }

    explicit DoubleBuffer(const T& data) {
        buffers_[0] = std::make_shared<T>(data);
        buffers_[1] = std::make_shared<T>(data);
        read_idx_ = 0;
    }

 public:
    /**
     * @brief Get read_buffer and you can read it without restriction
     * 
     * @return std::shared_ptr<T> 
     */
    std::shared_ptr<T> Load() {
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
        std::shared_ptr<T> write_buffer = monopolizer_writer_buffer();

        // update both two buffers
        updater(*write_buffer);
        this->swap_buffer();
        updater(*write_buffer);
    }

    /**
     * @brief Reset the value of write buffer
     * 
     * @param data 
     */
    void Reset(const T& data) {
        // use std::mutex to update exclusively in multiple writer thread scenarios
        std::lock_guard<std::mutex> lock(write_mtx_);
        std::shared_ptr<T> write_buffer = monopolizer_writer_buffer();

        // reset both two buffers
        *write_buffer.get() = data;
        this->swap_buffer();
        *write_buffer.get() = data;
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