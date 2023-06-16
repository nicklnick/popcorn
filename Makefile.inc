# Compiler
CC := gcc

# Compiler flags
CFLAGS := -Wall -std=c11 -pedantic -fsanitize=address -pthread -D_POSIX_C_SOURCE=200112L

# Source files directory
SRC_DIR := src

# Binary output directory
BIN_DIR := bin