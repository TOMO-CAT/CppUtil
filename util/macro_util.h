#pragma once

#include <cstring>
#include <string>

/**
 * printf with debug info
 */
#define printf2console(fmt, args...) \
do { \
    printf("[%s:%d][%s]" fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args); \
    fflush(stdout); \
} while (0) \

/**
 * perror with debug info
 */
#define perror2console(str) \
do { \
    char buff[50]; \
    snprintf(buff, sizeof(buff), "[%s:%d][%s]",  __FILE__, __LINE__, __FUNCTION__); \
    std::string new_str = std::string(buff) + str; \
    perror(new_str.c_str()); \
} while (0) \

