# double buffer

## 简介

双 buffer 也被称为 PingPang Buffer。

它解决的是这样一个问题：在代码中可能存在一些需要热更新的配置，但是在更新过程中会被不停地读取。如果使用读写锁的话一方面存在性能问题，另一方面配置可能一直更新不了（读线程一直持有锁）。

它实现的大致原理如下：

1. 在内存中维护两个 buffer，一个是只读的（read buffer），一个是给写线程更新的（write buffer）
2. 读线程永远只从 read buffer 读取数据，在读之前令引用计数 +1，读之后令引用计数-1。
3. 写线程检查 write buffer 的引用计数，当引用计数为 1 时就会尝试独占 write buffer，抢到了就会开始 update
4. 写线程 update 结束后会切换 read buffer 和 write buffer，然后再释放 write buffer 的引用计数

## 注意事项

注意理论上是不存在“无锁”双 buffer 的，如果有多个写线程，且多写少读场景下，触发 coredump 的概率更高。如果想复现的话，你可以在 `test.cpp` 增加写线程的数目，并在 `double_buffer.h` 中缩短 `update_write_buffer` 和 `reset_write_buffer` 的 sleep 时间。

另外需要注意的是写线程往往是阻塞的，即双 buffer 并不承诺 `Update()` 或者 `Reset()` 函数执行的时间，这意味着你最好以异步的方式调用这两个函数，而且写线程越多阻塞时间越长，最好是“一写多读”的场景。

## 用法

#### 1. 构造函数

提供了三种构造函数，方便使用：

```c++
template <typename T>
class DoubleBuffer {
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
...
};
```

#### 2. 读取数据

读是安全的，你可以将拿到的 `shared_ptr` 传递给任意位置，但是请切记，如果该 `shared_ptr` 未被归还，那么你永远无法读到更新的数据。（这意味着写线程会阻塞在等待该 `shared_ptr` 引用计数器置为 1 的条件上）。

```c++
DoubleBuffer<T> db;
std::shared_ptr<T> data_sptr = db.Load();
```

#### 3. 更新数据

有两个更新数据的接口：

```c++
// 增量更新数据, 需要用户自定义的闭包函数
void DoubleBuffer::Update(const UpdaterFunc& updater);

// 全量更新数据, 底层会进行拷贝, 这意味着 Update 性能更优
void DoubleBuffer::Reset(const T& data);
```

## 例子

在 `test.cpp` 中我们起了 20 个读线程以 QPS 一万的速度去读一个 `std::map<int,std::string>`，同时起了两个写线程分别以一秒一次的速度 update 和 reset 该数据，这基本模拟了使用双 buffer 进行配置热更新的极端场景。