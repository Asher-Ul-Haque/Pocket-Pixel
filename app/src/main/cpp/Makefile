# Directories
GAMEBOY_SRC := GameBoy/implement
FORGE_SRC   := ForgeLibrary/implement
GAMEBOY_INC := GameBoy/include
FORGE_INC   := ForgeLibrary/include
BUILD_DIR   := build

# Compilers
CXX := clang++
CC  := clang

# Compiler flags
CXXFLAGS := -I$(GAMEBOY_INC) -I$(FORGE_INC) -I/usr/include/SFML -std=c++17 -DDEBUG -g -O0
CFLAGS   := -I$(GAMEBOY_INC) -I$(FORGE_INC) -std=c11

# SFML link flags
SFML_LIBS := -lsfml-graphics -lsfml-window -lsfml-system

# Source files
CPP_SRC := $(shell find $(GAMEBOY_SRC) $(FORGE_SRC) -name '*.cpp')
C_SRC   := $(shell find $(FORGE_SRC) -name '*.c')
MAIN_SRC := LocalBridge.cpp
CORE_SRC := GameBoyCore.cpp

# Object files
CPP_OBJ := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CPP_SRC))
C_OBJ   := $(patsubst %.c,   $(BUILD_DIR)/%.o, $(C_SRC))
MAIN_OBJ := $(BUILD_DIR)/LocalBridge.o
CORE_OBJ := $(BUILD_DIR)/GameBoyCore.o
OBJ_FILES := $(CPP_OBJ) $(C_OBJ) $(MAIN_OBJ) $(CORE_OBJ)

# Executable
OUTPUT := gameboy

# Default target
all: $(OUTPUT)

# Compile C++ sources
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling C++: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C sources
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "Compiling C:   $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compile main file
$(MAIN_OBJ): $(MAIN_SRC)
	@mkdir -p $(dir $@)
	@echo "Compiling Main: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile GameBoyCore.cpp
$(CORE_OBJ): $(CORE_SRC)
	@mkdir -p $(dir $@)
	@echo "Compiling Core: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link all object files into executable
$(OUTPUT): $(OBJ_FILES)
	@echo "Linking executable: $@"
	$(CXX) $(OBJ_FILES) -o $@ $(SFML_LIBS)
	@echo "Build complete -> $(OUTPUT)"

# Clean
clean:
	rm -rf $(BUILD_DIR) $(OUTPUT)

# Debug helper
print-debug:
	@echo CPP_SRC: $(CPP_SRC)
	@echo C_SRC: $(C_SRC)
	@echo OBJ_FILES: $(OBJ_FILES)

.PHONY: all clean print-debug
