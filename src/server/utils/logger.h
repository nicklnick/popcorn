#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>

#define STRINGIFY_LEVEL(x)                                                     \
    case x:                                                                    \
        return #x;

typedef enum LOG_LEVEL {
    INFO,
    DEBUG,
    ERROR,
    FATAL
} LOG_LEVEL;

extern LOG_LEVEL curr_level;

void set_log_level(LOG_LEVEL level);

char *level_description(LOG_LEVEL level);

#define log(level, fmt, ...)                                                   \
    {                                                                          \
        if (level >= curr_level) {                                             \
            FILE *exit = level >= ERROR ? stderr : stdout;               \
            fprintf(exit, "%s: %s:%d, ", level_description(level), __FILE__,    \
                    __LINE__);                                                 \
            fprintf(exit, fmt, ##__VA_ARGS__);                                 \
            fprintf(exit, "\n");                                               \
        }                                                                      \
        if (level == FATAL)                                                \
            exit(1);                                                           \
    }

#endif // LOGGER_H
