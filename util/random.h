#pragma once

#include <cstdlib>

namespace util {

static unsigned int seed = 123;

/**
 * @brief return random value in the range [min, max]
 * 
 * @param min 
 * @param max 
 * @return int 
 */
inline int rand_int(int min, int max) {
    return min + rand_r(&seed) % (max - min);
}

/**
 * @brief return random value in the range [0, max]
 * 
 * @param max 
 * @return int 
 */
inline int rand_int(int max) {
    return rand_r(&seed) % max;
}

}  // namespace util