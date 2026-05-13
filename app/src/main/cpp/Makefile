# - - - Directories
SRC_DIR         := src
INCLUDE_DIR     := include
TESTS_DIR       := tests
BUILD_DIR       := .build
BIN_DIR         := bin
DOCS_DIR        := docs
TEST_BIN_DIR    := $(BIN_DIR)/tests

# - - - Compilers
CC      := clang
CXX     := clang++
SDL3_CFLAGS := $(shell pkg-config --cflags sdl3)
SDL3_LIBS   := $(shell pkg-config --libs sdl3)

CFLAGS  := -I$(INCLUDE_DIR) $(SDL3_CFLAGS) -Wall -Werror -Wextra -Wpedantic -DDEBUG -O1 -MMD -MP 
CXXFLAGS:= -I$(INCLUDE_DIR) $(SDL3_CFLAGS) -Wall -Werror -Wextra -Wpedantic -DDEBUG -O1 -MMD -MP
LDFLAGS := $(SDL3_LIBS)
RPATH   :=

# - - - Colors
GREEN  := \033[0;32m
RED    := \033[0;31m
YELLOW := \033[1;33m
PURPLE := \033[1;35m
PINK   := \033[95m
BLUE   := \033[0;34m
RESET  := \033[0m

# - - - Root project name
ROOT_NAME := pixel_pocket_core

# - - - Main (FIXED)
MAIN_SRC := $(wildcard main.c main.cpp)
MAIN_OBJ := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(MAIN_SRC))
MAIN_OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(MAIN_OBJ))

# - - - Application sources (excluding main)
SRC_FILES_C    := $(shell git ls-files | grep -E "^$(SRC_DIR)/.*\.c$$")
SRC_FILES_CPP  := $(shell git ls-files | grep -E "^$(SRC_DIR)/.*\.cpp$$")
OBJ_FILES_C    := $(patsubst $(SRC_DIR)/%.c,  $(BUILD_DIR)/%.o, $(SRC_FILES_C))
OBJ_FILES_CPP  := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o, $(SRC_FILES_CPP))
OBJ_FILES      := $(OBJ_FILES_C) $(OBJ_FILES_CPP)

# - - - Tests
TEST_FILES_C   := $(shell find $(TESTS_DIR) -name "*.c")
TEST_FILES_CPP := $(shell find $(TESTS_DIR) -name "*.cpp")

# - - - Final executable
TARGET := $(BIN_DIR)/$(ROOT_NAME)

# - - - Default target
all: banner sources $(MAIN_OBJ) tests $(TARGET) docs finished

banner:
	@clear
	@echo "$(PURPLE) - - - | Building Project $(ROOT_NAME) | - - -$(RESET)"
	@echo ""
	@echo ""

sourceBanner:
	@echo "$(PINK)- - - Building Source - - -$(RESET)"
	@echo ""
	@echo ""

sources: sourceBanner $(OBJ_FILES)
	@echo "$(GREEN)- - - Finished Building Source - - -$(RESET)"
	@echo ""
	@echo ""

# - - - Compile main (NEW)
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo " |_ Compiling (C) : $<"
	@$(CC) $(CFLAGS) -c $< -o $@ >/dev/null 2>&1 \
		&& echo "    $(GREEN)✓ Success$(RESET)" \
		|| (echo "    $(RED)✗ Error compiling $<$(RESET)"; $(CC) $(CFLAGS) -c $< -o $@; exit 1)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo " |_ Compiling (C++) : $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@ >/dev/null 2>&1 \
		&& echo "    $(GREEN)✓ Success$(RESET)" \
		|| (echo "    $(RED)✗ Error compiling $<$(RESET)"; $(CXX) $(CXXFLAGS) -c $< -o $@; exit 1)

# - - - Compile src
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo " |_ Compiling (C) : $<"
	@$(CC) $(CFLAGS) -c $< -o $@ >/dev/null 2>&1 \
		&& echo "    $(GREEN)✓ Success$(RESET)" \
		|| (echo "    $(RED)✗ Error compiling $<$(RESET)"; $(CC) $(CFLAGS) -c $< -o $@; exit 1)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo " |_ Compiling (C++) : $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@ >/dev/null 2>&1 \
		&& echo "    $(GREEN)✓ Success$(RESET)" \
		|| (echo "    $(RED)✗ Error compiling $<$(RESET)"; $(CXX) $(CXXFLAGS) -c $< -o $@; exit 1)

# - - - Application build (FIXED)
$(TARGET): $(OBJ_FILES) $(MAIN_OBJ)
	@mkdir -p $(BIN_DIR)
	@echo "$(PINK)> Linking application $(TARGET)$(RESET)"
	@$(CXX) $^ -o $@ $(LDFLAGS) $(RPATH) \
		&& echo "$(GREEN)✓ Application built successfully$(RESET)" \
		|| (echo "$(RED)✗ Linking failed$(RESET)"; exit 1)
	@echo "$(GREEN)- - - Finished Building Application - - -$(RESET)"
	@echo ""

# - - - Tests build target (unchanged)
tests: sources
	@echo "$(BLUE)- - - Building Tests - - -$(RESET)"
	@mkdir -p $(TEST_BIN_DIR)
	@for test in $(TEST_FILES_C) $(TEST_FILES_CPP); do \
		name=$$(basename $$test | sed 's/\.[^.]*$$//'); \
		echo " |_ Linking test : $$test"; \
		if echo $$test | grep -q '\.c$$'; then \
			$(CC) $(CFLAGS) $$test $(OBJ_FILES) -o $(TEST_BIN_DIR)/$$name $(LDFLAGS) $(RPATH_TEST) >/dev/null 2>&1 \
				&& echo "    $(GREEN)✓ Built test: $(TEST_BIN_DIR)/$$name$(RESET)" \
				|| (echo "    $(RED)✗ Error building test $$test$(RESET)"; $(CC) $(CFLAGS) $$test $(OBJ_FILES) $(MAIN_OBJ) -o $(TEST_BIN_DIR)/$$name $(LDFLAGS) $(RPATH_TEST); exit 1); \
		else \
			$(CXX) $(CXXFLAGS) $$test $(OBJ_FILES) $(MAIN_OBJ) -o $(TEST_BIN_DIR)/$$name $(LDFLAGS) $(RPATH) >/dev/null 2>&1 \
				&& echo "    $(GREEN)✓ Built test: $(TEST_BIN_DIR)/$$name$(RESET)" \
				|| (echo "    $(RED)✗ Error building test $$test$(RESET)"; $(CXX) $(CXXFLAGS) $$test $(OBJ_FILES) $(MAIN_OBJ) -o $(TEST_BIN_DIR)/$$name $(LDFLAGS) $(RPATH_TEST); exit 1); \
		fi; \
	done
	@echo "$(GREEN)- - - Finished Building Tests - - -$(RESET)"
	@echo ""
	@echo ""

finished:
	@echo ""
	@echo "$(GREEN)- - - Built Project : $(ROOT_NAME) - - -$(RESET)"
	@echo ""

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(DOCS_DIR)

remake: clean all

remake-sources:
	@echo "$(PINK)- - - Remaking Sources - - -$(RESET)"
	@rm -rf $(BUILD_DIR)
	$(MAKE) sources
	@echo "$(GREEN)- - - Finished Remaking Sources - - -$(RESET)"

remake-target:
	@echo "$(PURPLE)- - - Remaking Application - - -$(RESET)"
	@rm -f $(TARGET)
	$(MAKE) $(TARGET)
	@echo "$(GREEN)- - - Finished Remaking Application - - -$(RESET)"

remake-tests:
	@echo "$(BLUE)- - - Remaking Tests - - -$(RESET)"
	@rm -rf $(TEST_BIN_DIR)
	$(MAKE) tests
	@echo "$(GREEN)- - - Finished Remaking Tests - - -$(RESET)"

remake-docs:
	@echo "$(YELLOW)- - - Remaking Documentation - - -$(RESET)"
	@rm -rf $(DOCS_DIR)
	$(MAKE) docs
	@echo "$(GREEN)- - - Finished Remaking Documentation - - -$(RESET)"

run:
	@clear
	@$(TARGET)

compileRun: $(TARGET)
	@clear
	@$(TARGET)

docs:
	@echo "$(BLUE) - - - Building Doxygen Documentation - - -$(RESET)"
	@doxygen Doxyfile >/dev/null 2>&1 \
		&& echo "$(GREEN)✓ Documentation built successfully$(RESET)" \
		|| (echo "$(RED)✗ Error building documentation$(RESET)"; doxygen Doxyfile; exit 1)
	@echo "$(GREEN)- - - Finished Building Documentation - - -$(RESET)"
	@echo ""

-include $(OBJ_FILES:.o=.d) $(MAIN_OBJ:.o=.d)

.PHONY: all clean banner finished sources tests docs remake \
        remake-sources remake-target remake-tests
