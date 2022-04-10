#include <vector>
#include <string>
#include <sstream>
#include "string_util.h"

namespace util {

void string_split(const std::string& str, char delimter, std::vector<std::string>& res) {
    std::stringstream ss(str);
    while (ss.good()) {
        std::string elem;
        std::getline(ss, elem, delimter);
        res.push_back(trim(elem));
    }
}

}  // namespace util