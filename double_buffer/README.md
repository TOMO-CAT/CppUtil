# “无锁”双 buffer

## 简介

> 源码地址：<https://github.com/TOMO-CAT/CppUtil/tree/main/util/double_buffer>

双 buffer 也被称为 PingPang Buffer。

它解决的是这样一个问题：在代码中可能存在一些需要热更新的配置，但是在更新过程中会被不停地读取。如果使用读写锁的话一方面存在性能问题，另一方面配置可能一直更新不了（读线程一直持有锁）。

## 原理

它实现的大致原理如下：

1. 在内存中维护两个 buffer，一个是只读的（read buffer），一个是给写线程更新的（write buffer）
2. 读线程永远只从 read buffer 读取数据，在读之前令引用计数 +1，读之后令引用计数 -1
3. 写线程检查 write buffer 的引用计数，当引用计数为 1 时就会尝试独占 write buffer，抢到了就会开始 update
4. 写线程 update 结束后会切换 read buffer 和 write buffer，然后再释放 write buffer 的引用计数

## 问题及解决思路

一般而言双 buffer 都是应用在“一写多读”的场景，其中的写线程往往是异步且低频的，因此可以容忍一定的写阻塞。所以我们只需要做到无锁读即可，这中间会遇到一些问题：

### 1. 写线程需要执行两次写操作

由于存在两份 buffer，这意味每次写线程执行写操作时都必须执行两次，保证两份 buffer 都能更新，否则就会存在版本不一致的问题。

### 2. 并发写的数据不一致和 coredump 问题

写线程的更新频率很低，但我并不想限制用户并发写。这主要是业务中异步写线程往往有两个：

* **update 线程**：“高频”增量更新数据（这里的高频仅仅是相对于 reset 线程而言，相比于 read 线程频率依然很低），一般是秒级别增量更新数据
* **reset 线程**：低频全量更新数据，一般是小时级别甚至天级别全量更新数据

并发写存在两个问题，一个是两个 buffer 的数据不一致问题。假设两个写线程 writer1 和 writer2 同时调用 Update 函数执行不同的增量更新逻辑（Update 底层需要更新两次以保证两份 buffer 都能更新），那么可能出现两份 buffer 会分别执行两次完全相同的增量更新逻辑，从而导致两份 buffer 数据不一致。

第二个问题是并发写会导致触发 coredump 的概率更高，这是因为判断 writer buffer 引用计数为 1 和独占 write buffer 这两步不是原子的，有可能两个写线程同时以为自己独占了写 buffer。

解决这个问题的思路很简单，给写线程加上互斥锁将写任务转成串行的即可，毕竟这不会影响我们的无锁读。

### 3. 写线程的阻塞问题

写线程往往是阻塞的，即双 buffer 并不承诺 `Update()` 或者 `Reset()` 函数执行的时间，这意味着你 **最好以异步的方式调用这两个函数**。世上没有免费的午餐，计算机世界里到处充满了 tradeoff（例如空间换时间），可以当作是“无锁”读需要付出的代价。

### 4. 极端场景下的 coredump 问题

即使我们加了一把互斥锁实现了写线程串行执行，那这个双 buffer 就不可能会挂了吗？

考虑一下这个极端的场景：

写线程更新比较频繁，读线程刚拿到 read buffer 的 `shared_ptr`，此时 read buffer 的引用计数还来不及 + 1，写线程立马 swap 并尝试独占 write buffer，那么它就会拿到 swap 之前的 read buffer 从而导致并发读写。

解决方法有两个：

1. 每次 swap 两个 buffer 后加一个 sleep 时间（例如 1ms），降低这种极端场景的出现概率
2. 使用 atomic 语义的 `shared_ptr`，这在不同的 C++ 版本有不同的实现
    * C++11 之前可以用 `boost::shared_ptr` 搭配 `boost::atomic_*` 函数族使用
    * C++11 可以使用 `std::atomic_*` 函数族直接对 `std::shared_ptr<T>` 进行原子操作
    * C++20 后可以使用 `std::atomic<std::shared_ptr<T>>` 原子类型

根据 [cpp reference](https://en.cppreference.com/w/cpp/memory/shared_ptr/atomic) 中的解释，如果多个执行线程在没有同步的情况下并发访问同一个 `std::shared_ptr` 对象，并且这些访问中的任何一个使用 `shared_ptr` 的非常量成员函数，则将发生数据竞争。除非所有此类访问都是通过 `std::atomic_load` 和 `std::atomic_store` 这类函数执行的。可以猜想 `std::shared_ptr` 的原子操作底层应该是通过自旋锁实现的，这违背了我们“无锁”双 buffer 的初衷。因此我们选择第一种方法，这就意味着理论上这种实现还是线程不安全的。

## 用法

### 1. 构造函数

提供了三种构造函数，方便使用：

```c++
template <typename T>
class DoubleBuffer {
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
...
};
```

### 2. 读取数据

读是安全的，你可以将拿到的 `shared_ptr` 传递给任意位置，但是请切记，如果该 `shared_ptr` 未被归还，那么你永远无法读到更新的数据。（这意味着写线程会阻塞在等待该 `shared_ptr` 引用计数器置为 1 的条件上）。

```c++
DoubleBuffer<T> db;
std::shared_ptr<T> data_sptr = db.Load();
```

### 3. 更新数据

有两个更新数据的接口：

```c++
// 增量更新数据, 需要用户自定义的闭包函数
void DoubleBuffer::Update(const UpdaterFunc& updater);

// 全量更新数据, 底层会进行拷贝, 这意味着 Update 性能更优
void DoubleBuffer::Reset(const T& data);
```
