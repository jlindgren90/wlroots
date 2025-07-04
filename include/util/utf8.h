#ifndef UTIL_UTF8_H
#define UTIL_UTF8_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return true if and only if the string is a valid UTF-8 sequence.
 */
bool is_utf8(const char *string);

#ifdef __cplusplus
}
#endif

#endif
