#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace util {

inline std::string& ltrim(std::string* const s) {
  s->erase(s->begin(), std::find_if(s->begin(), s->end(), std::not1(std::ptr_fun(isspace))));
  return *s;
}

inline std::string& rtrim(std::string* const s) {
  s->erase(find_if(s->rbegin(), s->rend(), std::not1(std::ptr_fun(isspace))).base(), s->end());
  return *s;
}

inline std::string& trim(std::string* const s) {
  return ltrim(&rtrim(s));
}

void string_split(const std::string& str, char delimter, std::vector<std::string>* const res);

}  // namespace util
