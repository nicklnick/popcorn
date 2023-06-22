include Makefile.inc

# Source files
SRCS := $(shell find . -name "*.c")

# Object files
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRCS))

# Binary output
TARGET := $(BIN_DIR)/server

# Default target
all: $(TARGET)

# Compile and link the source files
$(TARGET): $(SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Clean the generated files
clean:
	rm -rf $(BIN_DIR)