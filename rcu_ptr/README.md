# 基于 RCU 实现无锁并发读写

## 前言

一般而言并发读写一个结构体我会使用三种方式：

* 读写锁
* 双 buffer
* RCU（read-copy update）

关于双 buffer 的可以回顾一下以前我写的文章：

> [C++实现“无锁”双buffer](https://zhuanlan.zhihu.com/p/581299675)

## 如何选择

在实际开发中这三个各有优缺点：

* **读写锁**：实现简单，但高并发锁争抢严重导致性能不佳，一般用于并发度不高的多线程读写场景，并且读写临界区越大并发压力越大
* **双 buffer**：
  * “ 无锁”双 buffer 可以实现无锁读，因此读性能极佳，但是存在写阻塞问题，所以一般用于“一写多读”配置热更新场景，且写线程往往是异步的对阻塞时长容忍度高
  * 线程安全双 buffer 读写均加锁了，但是临界区较小，相比于读写锁而言可抗的并发度更高
* **RCU**：实现比双 buffer 简单，并且可以实现无锁读写，但是每次更新数据都需要拷贝原始数据，在 **高频写且数据较大** 时存在大量的内存申请、内存拷贝和垃圾回收开销

除此之外，RCU 可以保证读操作都可以读到最新写入的数据，因此在数据实时性要求较高的场景是更佳的选择。

## 例子

> 参考 <https://martong.github.io/high-level-cpp-rcu_informatics_2017.pdf> 中的例子。

假设我们需要并发读写一个 `std::vector`，最粗暴的方式便是直接加互斥锁：

```c++
template <typename T>
class ThreadSafeVector {
 public:
    // read operation
    int Sum() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return std::accumulate(data_.begin(), data_.end(), 0);
    }

    // write operation
    void Add(int i) {
        std::lock_guard<std::mutex> lock(mtx_);
        data_.push_back(i);
    }

 private:
    std::vector<T> data_;
    mutable std::mutex mtx_;
};
```

这种使用互斥锁保护共享数据的方式在多线程争抢严重的情况下表现不佳，尤其是“多读少写”的场景下更是浪费 CPU（考虑一下极端场景只读不写，其实是不需要加锁的）。

常见的做法是考虑使用读写锁来提高“多读少写”场景下的并发性能：

```c++
template <typename T>
class ThreadSafeVector {
 public:
    // read operation
    int Sum() const {
        boost::shared_lock<boost::shared_mutex> rl(rw_lock_);
        return std::accumulate(data_.begin(), data_.end(), 0);
    }

    // write operation
    void Add(int i) {
        boost::unique_lock<boost::shared_mutex> wl(rw_lock_);
        data_.push_back(i);
    }

 private:
    std::vector<T> data_;
    mutable boost::shared_mutex rw_lock_;
};
```

但是读写锁依然无法解决并发写的问题，在频繁写场景下难以提高服务并发度。

有一种常见的思路就是用智能指针包装共享数据结构，只对智能指针的 Load 和 Store 加锁从而减少临界区大小。另外我们需要深拷贝来保证底层数据的并发安全：

```c++
template <typename T>
class ThreadSafeVector {
 public:
    ThreadSafeVector() : data_(std::make_shared<std::vector<T>>()) {}

    // read operation
    int Sum() const {
        auto data = safe_load();
        // assume processing the data takes longer than copying std::shared_ptr
        // otherwise, wo should lock the processing instead of the copying
        return std::accumulate(data->begin(), data->end(), 0);
    }

    // write operation
    void Add(int i) {
        auto data = safe_load();
        auto data_deep_copy = std::make_shared<std::vector<int>>(*data);
        data_deep_copy->push_back(i);
        safe_store(data_deep_copy);
    }

 private:
    std::shared_ptr<std::vector<T>> data_;
    mutable std::mutex mtx_;

 private:
    std::shared_ptr<std::vector<T>> safe_load() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return data_;
    }

    void safe_store(std::shared_ptr<std::vector<T>> data) {
        std::lock_guard<std::mutex> lock(mtx_);
        data_ = data;
    }
};
```

但是这种写法存在另一个问题，如果有多个写线程并发更新数据的话，我们可能会丢失一些更新任务。我们需要使用 `atomic_compare_exchange` 配合自旋操作使得写线程的每一次 update 操作都不会丢失：

```c++
#include <atomic>
#include <memory>
#include <numeric>
#include <vector>

template <typename T>
class ThreadSafeVector {
 public:
    ThreadSafeVector() : data_(std::make_shared<std::vector<T>>()) {}

    // read operation
    int Sum() const {
        auto data_copy = std::atomic_load(&data_);
        return std::accumulate(data_copy->begin(), data_copy->end(), 0);
    }

    // write operation
    void Add(int i) {
        auto data_copy = std::atomic_load(&data_);
        bool is_exchange = false;
        while (!is_exchange) {
            auto data_deep_copy = std::make_shared<std::vector<int>>(*data_copy);
            data_deep_copy->push_back(i);
            // is_exchange = std::atomic_compare_exchange_strong(&data_, &data_copy, data_deep_copy);
            is_exchange = std::atomic_compare_exchange_strong(&data_, &data_copy, std::move(data_deep_copy));
        }
    }

 private:
    std::shared_ptr<std::vector<T>> data_;
};
```

有一些优化细节可以注意一下：

1. 我们可以使用 `std::shared_ptr<T>` 的移动构造函数来节省掉 `atomic_compare_exchange_strong` 函数中引用计数的一次递增和递减操作。
2. 我们可以使用 `atomic_compare_exchange_weak` 来替换 `atomic_compare_exchange_strong`，在一些平台上可以取得一些性能提升，但是它可能失败从而带来更多次的深拷贝成本，可以根据具体的服务场景进行测试选择最佳的实现方式

## RCU 原理

RCU 全称是 Read-Copy Update，指的是并发读写数据时读线程不加锁读取数据，而写线程先拷贝一份原始数据的副本，修改完副本后再更新数据指针。由于指针的读写是原子的，因此 RCU 可以实现无锁读和无锁写，相比于读写锁和双 buffer 而言可以抗更高的并发度。

由于每次写操作（无论是轻量的 update 还是全量的 reset 操作）都需要重新拷贝一份新的数据副本，因此如果不对这些历史副本及时清理就会出现 OOM 的问题。

## RCU 的 Go 实现

### 1. 简介

由于 Go 语言自带 GC，因此我们无需操心数据副本，可以用极其简单的方式实现无锁 RCU。

### 2. 代码

以 map 的并发读写为例：

```go
package main

import (
 "fmt"
 "math/rand"
 "strconv"
 "sync/atomic"
 "time"
)

const (
 readGoroutineCount  = 20
 writeGoroutineCount = 2

 readQPS  = 10000
 writeQPS = 4
)

var data atomic.Value

func read() {
 m := data.Load().(map[int]string)
 mLen := len(m)
 if rand.Intn(readQPS) == 0 {
  fmt.Printf("[read] data len:%d\n", mLen)
 }
}

func write() {
 m := make(map[int]string)
 keyCnt := 1000 + rand.Intn(1000)
 for i := 0; i < keyCnt; i++ {
  m[rand.Intn(10000)] = strconv.Itoa(i)
 }
 data.Store(m)

 if rand.Intn(writeQPS) == 0 {
  fmt.Printf("[write] write %d keys\n", keyCnt)
 }
}

func main() {
 data.Store(make(map[int]string))

 // read operation
 for i := 0; i < readGoroutineCount; i++ {
  go func() {
   for {
    read()
    time.Sleep(time.Millisecond * time.Duration(1000*readGoroutineCount/readQPS))
   }
  }()
 }

 for i := 0; i < writeGoroutineCount; i++ {
  go func() {
   for {
    write()
    time.Sleep(time.Millisecond * time.Duration(1000*writeGoroutineCount/writeQPS))
   }

  }()
 }

 time.Sleep(5 * time.Minute)
}

```

## RCU 的 C++ 实现

C++ 中只能自己手动管理内存或者使用引用计数的 `shared_ptr`，前者可以实现指针的原子读写但会引入额外的 GC 成本，后者自动管理内存但是读写智能指针需要加锁。

这里我们使用后者，在不同的 C++ 版本中可以使用不同方法保障 `std::shared_ptr` 原子读写（这里的原子指的是原子语义，例如用自旋锁实现，本质上还是加锁了）：

* C++11 之前可以用 `boost::shared_ptr` 搭配 `boost::atomic_*` 函数族使用
* C++11 可以使用 `std::atomic_*` 函数族直接对 `std::shared_ptr<T>` 进行原子操作
* C++20 后可以使用 `std::atomic<std::shared_ptr<T>>` 原子类型

> 最新代码地址：<https://github.com/TOMO-CAT/CppUtil/tree/main/util/rcu_ptr>

## Reference

[1] <https://pkg.go.dev/mosn.io/mosn/pkg/rcu>

[2] <https://zhuanlan.zhihu.com/p/386422612>

[3] <https://martong.github.io/high-level-cpp-rcu_informatics_2017.pdf>

[4] <https://juejin.cn/post/6844904130989801479>
