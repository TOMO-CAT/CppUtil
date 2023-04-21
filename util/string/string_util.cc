#include "util/string/string_util.h"

#include <sstream>
#include <string>
#include <vector>

namespace util {

void string_split(const std::string& str, char delimter, std::vector<std::string>* const res) {
  std::stringstream ss(str);
  while (ss.good()) {
    std::string elem;
    std::getline(ss, elem, delimter);
    res->push_back(trim(&elem));
  }
}

}  // namespace util
