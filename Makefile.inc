# Compiler
CC := gcc

# Compiler flags
CFLAGS := -Wall -Wextra -Wno-unused-parameter -Wno-newline-eof -Wno-implicit-fallthrough -std=c11 -pedantic-errors -g -fsanitize=address -pthread -D_POSIX_C_SOURCE=200112L

# Source files directory
SRC_DIR := src

# Binary output directory
BIN_DIR := bin