#include "logger/backtrace.h"

#include <backtrace.h>
#include <cxxabi.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>

namespace logger {

namespace {

constexpr uint32_t kDemangleBufferSize = 4096;
constexpr uint32_t kMaxStackFrames = 128;

struct BacktraceContext {
  StackDumper* sd = nullptr;
  uint32_t frame_depth = 0;
  std::vector<std::string>* res;
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

bool StackDumper::Dump(std::vector<std::string>* const stack_frames) {
  struct BacktraceContext bc = {
      .sd = this,
      .frame_depth = 0,
      .res = stack_frames,
  };
  struct backtrace_state* state = ::backtrace_create_state(exec_path_.c_str(), 1, this->ErrorCallback, nullptr);
  ::backtrace_full(state, skip_, this->BacktraceCallback, this->ErrorCallback, reinterpret_cast<void*>(&bc));
  return true;
}

void StackDumper::ErrorCallback(void* data, const char* msg, int errnum) {
  std::cerr << msg << std::endl;
}

int StackDumper::BacktraceCallback(void* data, uintptr_t pc, const char* file, int line, const char* func) {
  BacktraceContext* ptr_bc = reinterpret_cast<BacktraceContext*>(data);
  return ptr_bc->sd->Backtrace(file, line, func, &ptr_bc->frame_depth, ptr_bc->res);
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

int StackDumper::Backtrace(const char* file, int line, const char* func, uint32_t* const count,
                           std::vector<std::string>* const stacks) {
  if (!file && !func) {
    return 0;
  }

  std::ostringstream oss;
  if (*count >= kMaxStackFrames) {
    oss << '#' << (*count) << " [reach max stack frames, truncated]";
    stacks->emplace_back(oss.str());
    return -1;
  }

  char* p = nullptr;
  if (func) {
    p = this->Demangle(func);
    if (p != nullptr) {
      func = p;
    }
  }

  oss << '#' << (*count) << " [" << (file ? file : "???") << ':' << line << "][" << (func ? func : "???") << ']';
  (*count)++;
  stacks->emplace_back(oss.str());
  if (p && p != demangle_buff_) {
    ::free(p);
  }
  return 0;
}

}  // namespace logger
