#include <backtrace.h>
#include <cxxabi.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

static constexpr size_t size = 4096;
char buffer[size] = {0};

char* demangle(const char* name) {
  int status = 0;
  size_t n = size;
  char* p = abi::__cxa_demangle(name, buffer, &n, &status);
  if (p && p != buffer) {
    std::cerr << "abi::__cxa_demangle: buffer reallocated.." << std::endl;
    memset(buffer, 0, size);
  }
  return p;
}

std::string exepath() {
  char buf[4096] = {0};
  int r = static_cast<int>(readlink("/proc/self/exe", buf, 4096));
  return r > 0 ? std::string(buf, r) : std::string();
}

void error_callback(void* data, const char* msg, int errnum) {
  std::cerr << msg << std::endl;
}

int backtrace(const char* file, int line, const char* func, int* const count) {
  if (!file && !func) {
    return 0;
  }
  char* p = nullptr;
  if (func) {
    p = demangle(func);
    if (p) {
      func = p;
    }
  }

  std::cout << '#' << (*count) << " in " << (func ? func : "???") << " at " << (file ? file : "???") << ':' << line
            << '\n';
  (*count)++;
  if (p && p != buffer) {
    ::free(p);
  }
  return 0;
}

int backtrace_callback(void* data, uintptr_t pc, const char* file, int line, const char* func) {
  //   std::cout << "## file: " << file << std::endl;
  //   std::cout << "## line: " << line << std::endl;
  //   std::cout << "## func: " << func << std::endl;
  static int count = 0;
  return backtrace(file, line, func, &count);
}

void dump_stack() {
  // struct backtrace_state* backtrace_create_state(const char* filename, int threaded, void (*error_callback)(void*
  // data, const char* msg, int errnum), void* data);
  //
  // @param filename: 指定文件的可执行文件路径
  // @param threaded: 是否使用多线程
  // @param error_callback: 回调函数指针, 用于处理错误信息
  // @param data: 指向任意类型数据的指针, 用于传递给 error_callback 函数
  struct backtrace_state* state = ::backtrace_create_state(exepath().c_str(), 1, error_callback, NULL);
  const int skip = 0;
  ::backtrace_full(state, skip, backtrace_callback, error_callback, nullptr);
}

void bar() {
  dump_stack();
}

void foo() {
  bar();
}

void cat() {
  foo();
}

// 如果开启优化选项 -O2, 那么打印的栈不全 (应该是函数过于简单编译器内联化了)
// #0 in main at logger/test/backtrace_test.cc:91
// #1 in __libc_start_call_main at ../sysdeps/nptl/libc_start_call_main.h:58
// #2 in __libc_start_main_impl at ../csu/libc-start.c:392
//
// 开启 -O0 的效果
// #0 in dump_stack() at logger/test/backtrace_test.cc:73
// #1 in bar() at logger/test/backtrace_test.cc:77
// #2 in foo() at logger/test/backtrace_test.cc:81
// #3 in cat() at logger/test/backtrace_test.cc:85
// #4 in main at logger/test/backtrace_test.cc:111
// #5 in __libc_start_call_main at ../sysdeps/nptl/libc_start_call_main.h:58
// #6 in __libc_start_main_impl at ../csu/libc-start.c:392
//
// 开启 -O1 的效果
// # 0 in dump_stack() at logger / test / backtrace_test.cc : 73
// # 1 in bar() at logger / test / backtrace_test.cc : 77
// # 2 in foo() at logger / test / backtrace_test.cc : 81
// # 3 in cat() at logger / test / backtrace_test.cc : 85
// # 4 in main at logger / test / backtrace_test.cc : 111
// # 5 in __libc_start_call_main at../ sysdeps / nptl / libc_start_call_main.h : 58
// # 6 in __libc_start_main_impl at../ csu / libc - start.c : 392
int main() {
  cat();
}
