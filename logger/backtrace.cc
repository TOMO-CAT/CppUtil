#include "logger/backtrace.h"

#include <backtrace.h>
#include <cxxabi.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

namespace logger {

namespace {

constexpr uint32_t kDemangleBufferSize = 4096;

struct BacktraceContext {
  StackDumper* sd = nullptr;
  uint32_t frame_depth = 0;
};

}  // namespace

StackDumper::StackDumper(uint32_t skip) : skip_(skip) {
  constexpr uint32_t kBufferSize = 4096;
  char buf[kBufferSize] = {0};
  int r = static_cast<int>(::readlink("/proc/self/exe", buf, kBufferSize));
  exec_path_ = r > 0 ? std::string(buf, r) : "";

  demangle_buff_ = static_cast<char*>(::malloc(kDemangleBufferSize));
  ::memset(demangle_buff_, 0, kDemangleBufferSize);
}

StackDumper::~StackDumper() {
  if (demangle_buff_) {
    ::free(demangle_buff_);
    demangle_buff_ = nullptr;
  }
}

std::string StackDumper::Dump() {
  struct BacktraceContext bc;
  struct backtrace_state* state = ::backtrace_create_state(exec_path_.c_str(), 1, this->ErrorCallback, nullptr);
  ::backtrace_full(state, skip_, this->BacktraceCallback, this->ErrorCallback, reinterpret_cast<void*>(&bc));
  return stack_.str();
}

void StackDumper::ErrorCallback(void* data, const char* msg, int errnum) {
  std::cerr << msg << std::endl;
}

int StackDumper::BacktraceCallback(void* data, uintptr_t pc, const char* file, int line, const char* func) {
  BacktraceContext* ptr_bc = reinterpret_cast<BacktraceContext*>(data);
  return ptr_bc->sd->Backtrace(file, line, func, &ptr_bc->frame_depth);
}

char* StackDumper::Demangle(const char* name) {
  int status = 0;
  size_t n = demangle_buff_ ? kDemangleBufferSize : 0;
  char* p = abi::__cxa_demangle(name, demangle_buff_, &n, &status);
  if (p && p != demangle_buff_) {
    std::cerr << "abi::__cxa_demangle: buffer reallocated..";
    demangle_buff_ = nullptr;
  }
  return p;
}

int StackDumper::Backtrace(const char* file, int line, const char* func, uint32_t* const count) {
  if (!file && !func) {
    return 0;
  }
  char* p = nullptr;
  if (func) {
    p = this->Demangle(func);
    if (p != nullptr) {
      func = p;
    }
  }

  stack_ << '#' << count << " [" << (file ? file : "???") << ':' << line << "][" << (func ? func : "???") << "]\n";
  (*count)++;
  if (p && p != demangle_buff_) {
    ::free(p);
  }
  return 0;
}

}  // namespace logger
