#ifndef UTIL_TOKEN_H
#define UTIL_TOKEN_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Number of bytes used by a token, including the terminating zero byte.
 */
#define TOKEN_SIZE 33

/**
 * Generate a random token string.
 */
bool generate_token(char out[static TOKEN_SIZE]);

#ifdef __cplusplus
}
#endif

#endif
