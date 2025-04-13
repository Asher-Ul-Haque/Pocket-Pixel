# Compiler settings
CC = clang
CXX = clang++
CFLAGS = -Wall -I../../../native/
CXXFLAGS = -Wall -I../../../native/
LDFLAGS =

# Source directories
SRC_DIRS = ../../../native/ 

# Find all .c and .cpp files in the source directories
C_SRCS := $(shell find $(SRC_DIRS) -name '*.c')
CPP_SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
SRCS := $(C_SRCS) $(CPP_SRCS)

# Generate corresponding object file paths in the obj directory
OBJS := $(patsubst %.c,obj/%.o,$(C_SRCS)) $(patsubst %.cpp,obj/%.o,$(CPP_SRCS))

# Output directories
OBJ_DIR = obj
BIN_DIR = bin
OUT_BIN = $(BIN_DIR)/emu

# Default target
all: $(OUT_BIN)

# Rule to create the output binary
$(OUT_BIN): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJS) -o $(OUT_BIN) $(LDFLAGS)

# Pattern rule to compile .c files
obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule to compile .cpp files
obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
