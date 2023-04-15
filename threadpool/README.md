# threadpool

## 简介

C++11特性线程池。

* 只有头文件
* 支持同步任务
* 支持任意参数类型的`Task`

## 使用方法

### 1. 全局线程池 + 异步任务

创建一个ThreadPool的全局变量，将所有需要异步执行的任务丢到该线程池中即可：

```c++
#include <unistd.h>
#include "threadpool.h"

// 全局异步线程池
ThreadPool g_threadpool2(20);

int main() {
    // 执行异步任务
    g_threadpool2.Enqueue(
        [] {
            sleep(1);
            printf("async task done\n");
        });
    return 0;
}
```

编译运行：

```bash
$g++ -g test.cpp -o test -std=c++11 -lpthread
$./test 
async task done
```

### 2. 全局线程池 + 同步任务

创建一个ThreadPool的全局变量并添加同步任务，通过`std::future`的`wait()`方法阻塞等待同步结果，也可以使用`get()`方法获取到函数返回值。

```c++
#include <unistd.h>
#include <memory>
#include "threadpool.h"

// 全局异步线程池
ThreadPool g_threadpool2(20);

int main() {
    // 创建同步任务
    auto res = g_threadpool2.Enqueue(
        [] {
            sleep(1);
            printf("sync task done\n");
        });

    // 阻塞等待同步结果
    res.wait();

    return 0;
}
```

编译运行：

```c++
$g++ -g test.cpp -o test -std=c++11 -lpthread
$./test 
sync task done
```

### 3. 局部线程池实现并发同步

创建一个临时ThreadPool，利用其析构函数完成并发同步任务：
> 需要注意的是，这种用法已经脱离了线程池的初衷（避免处理短时间任务时创建与销毁线程的代价），它的主要用途是实现「多线程并发」，常用于并发多个IO请求并等待同步结果。

考虑这个场景：代码中仅在某种特殊场景（极少触发）下需要并发请求多个http链接，一方面我们不希望这些请求影响到进程的业务线程池，另一方面我们又不想单独为这个场景创建一个全局线程池使其大部分时间都在空跑。

这种用法解决了我们「临时创建线程+执行并行任务+销毁线程」的局部并发问题，避免我们直接在用户代码处直接创建线程。

```c++
#include <unistd.h>
#include <memory>
#include "threadpool.h"


int main() {
    // 创建并发度为5的局部线程池
    std::shared_ptr<ThreadPool> threadpool = std::make_shared<ThreadPool>(5);

    // 创建30个异步任务
    for (int i = 0; i < 30; i++) {
        threadpool->Enqueue(
            [i] {
                sleep(1);
                printf("Info: thread %ld is working on task %d\n", (u_int64_t)pthread_self(), i);
            });
    }

    // 阻塞直至获取同步结果
    threadpool.reset();

    return 0;
}
```

编译运行：

```bash
$g++ -g test.cpp -o test -std=c++11 -lpthread
$./test 
Info: thread 139811129124608 is working on task 4
Info: thread 139811145910016 is working on task 2
Info: thread 139811137517312 is working on task 3
Info: thread 139811162695424 is working on task 0
Info: thread 139811154302720 is working on task 1
Info: thread 139811129124608 is working on task 5
Info: thread 139811137517312 is working on task 7
Info: thread 139811145910016 is working on task 6
Info: thread 139811162695424 is working on task 8
Info: thread 139811154302720 is working on task 9
Info: thread 139811129124608 is working on task 10
Info: thread 139811137517312 is working on task 11
Info: thread 139811162695424 is working on task 13
Info: thread 139811154302720 is working on task 14
Info: thread 139811145910016 is working on task 12
Info: thread 139811129124608 is working on task 15
Info: thread 139811137517312 is working on task 18
Info: thread 139811145910016 is working on task 19
Info: thread 139811162695424 is working on task 16
Info: thread 139811154302720 is working on task 17
Info: thread 139811129124608 is working on task 21
Info: thread 139811162695424 is working on task 23
Info: thread 139811154302720 is working on task 24
Info: thread 139811145910016 is working on task 22
Info: thread 139811137517312 is working on task 20
Info: thread 139811162695424 is working on task 25
Info: thread 139811154302720 is working on task 26
Info: thread 139811129124608 is working on task 27
Info: thread 139811137517312 is working on task 29
Info: thread 139811145910016 is working on task 28
```
