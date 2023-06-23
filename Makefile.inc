# Compiler
CC := gcc

# Compiler flags
CFLAGS := -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -std=c11 -pedantic-errors -g -fsanitize=address -pthread -Wno-gnu-zero-variadic-macro-arguments -D_POSIX_C_SOURCE=200112L -D_DEFAULT_SOURCE

# Binary output directory
BIN_DIR := ../../bin