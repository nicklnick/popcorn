#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#include <limits.h>

#define RESPONSE_LEN 256 // RFC 1939 - Section 3

#define MAILPATH_MAX (PATH_MAX - 2 * NAME_MAX) // path/username/filename

#define BUFFER_SIZE (12 * 1024)

#endif // _CONSTANTS_H
