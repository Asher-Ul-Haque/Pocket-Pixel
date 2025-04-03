# Compiler settings
CC = gcc
CXX = g++
CFLAGS = -Wall -I. -IForgeLib/include -IMachine/Parts -I Machine/Utils
CXXFLAGS = -Wall -I. -IForgeLib/include -I Machine/Parts -I Machine/Utils
LDFLAGS =

# Find all .c and .cpp files in the project
SRC_DIRS = ForgeLib/implement Machine/Parts Machine/Utils
C_SRC = $(wildcard $(foreach dir,$(SRC_DIRS),$(dir)/*.c))
CPP_SRC = $(wildcard *.cpp)

# Find all header files (optional, not necessary for compilation, but good for dependencies)
HEADERS = $(wildcard *.h) $(wildcard ForgeLib/include/*.h) $(wildcard Machine/Parts/*.h) $(wildcard Machine/Utils/*.h)

# Output directories
OBJ_DIR = obj
BIN_DIR = bin
OUT_BIN = emu

# Create object files from .c and .cpp sources
OBJ_C = $(C_SRC:.c=.o)
OBJ_CPP = $(CPP_SRC:.cpp=.o)

# List all object files
OBJ = $(OBJ_C) $(OBJ_CPP)

# Default target
all: $(OUT_BIN)

# Create object directory if it doesn't exist
$(OBJ_DIR)/%.o: %.c $(HEADERS) | $(OBJ_DIR)/$(dir $<)
$(OBJ_DIR)/%.o: %.cpp $(HEADERS) | $(OBJ_DIR)/$(dir $<)

$(OBJ_DIR)/$(dir $<):
	mkdir -p $@

# Link the object files to create the executable
$(OUT_BIN): $(OBJ)
	$(CXX) $(OBJ) -o $(OUT_BIN) $(LDFLAGS)

# Compile .cpp files
$(OBJ_DIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile .c files
$(OBJ_DIR)/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object files and executable
clean:
	rm -rf $(OBJ_DIR)/*.o $(OUT_BIN)

.PHONY: all clean
