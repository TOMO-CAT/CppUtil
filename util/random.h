#pragma once

#include <cstdlib>

namespace util {

/**
 * @brief return random value in the range [min, max]
 *
 * @param min
 * @param max
 * @return int
 */
inline int32_t RandInt(int32_t min, int32_t max) {
  static unsigned int seed = 123;
  return min + rand_r(&seed) % (max - min);
}

/**
 * @brief return random value in the range [0, max]
 *
 * @param max
 * @return int
 */
inline int32_t RandInt(int32_t max) {
  static unsigned int seed = 123;
  return rand_r(&seed) % max;
}

}  // namespace util
