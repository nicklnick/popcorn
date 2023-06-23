#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>

#include "../server_adt.h"

typedef enum LOG_LEVEL {
    INFO,
    DEBUG,
    ERROR,
    FATAL
} LOG_LEVEL;

extern LOG_LEVEL curr_level;

void set_log_level(LOG_LEVEL level);

char *level_description(LOG_LEVEL level);

/**
 * @brief Variadic logging, accepts variable arguments for format
 *
 * @note If level == FATAL, then close_server() and exit
 */
#define logv(level, fmt, ...)                                                  \
    {                                                                          \
        if (level >= curr_level) {                                             \
            FILE *exit = stderr;                                               \
            fprintf(exit, "%s: %s:%d, ", level_description(level), __FILE__,   \
                    __LINE__);                                                 \
            fprintf(exit, fmt, ##__VA_ARGS__);                                 \
            fprintf(exit, "\n");                                               \
        }                                                                      \
        if (level == FATAL) {                                                  \
            close_server();                                                    \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    }

/**
 * @brief Simple logging, if you want to format string use logv
 *
 * @note If level == FATAL, then close_server() and exit
 */
#define log(level, string)                                                     \
    {                                                                          \
        if (level >= curr_level) {                                             \
            FILE *exit = stderr;                                               \
            fprintf(exit, "%s: %s:%d, ", level_description(level), __FILE__,   \
                    __LINE__);                                                 \
            fprintf(exit, string);                                             \
            fprintf(exit, "\n");                                               \
        }                                                                      \
        if (level == FATAL) {                                                  \
            close_server();                                                    \
            exit(EXIT_FAILURE);                                                \
        } /* close server? */                                                  \
    }

#endif // LOGGER_H
