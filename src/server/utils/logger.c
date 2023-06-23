#include "logger.h"

#define STRINGIFY_LEVEL(x)                                                     \
    case x:                                                                    \
        return #x;

LOG_LEVEL curr_level = INFO;

void set_log_level(LOG_LEVEL level) {
    if (level >= INFO && level <= FATAL)
        curr_level = level;
}

char *level_description(LOG_LEVEL level) {
    switch (level) {
        STRINGIFY_LEVEL(INFO)
        STRINGIFY_LEVEL(DEBUG)
        STRINGIFY_LEVEL(ERROR)
        STRINGIFY_LEVEL(FATAL)
        default:
            return "";
    }
}
