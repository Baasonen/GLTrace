# --- Setup ---
CC        := gcc
TARGET    := glt
SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build

# Files
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEP := $(OBJ:.o=.d)

# --- OS Detection & Flags ---
ifeq ($(OS),Windows_NT)
    # Windows Settings
    GLFW_INC   ?= C:/libs/glfw/include
    GLFW_LIB   ?= C:/libs/glfw/lib
    
    CFLAGS     := -I$(INC_DIR) -I$(GLFW_INC) -Wall -MMD -MP -O2
    LDFLAGS    := -L$(GLFW_LIB)
    LIBS       := -lglfw3 -lopengl32 -lgdi32
    EXE_EXT    := .exe
    # Use 'rm -rf' if using Git Bash/MSYS2, otherwise 'rd /s /q'
    RM         := rm -rf
else
    # Linux / Unix Settings
    CFLAGS     := -I$(INC_DIR) -Wall -MMD -MP -O2
    LDFLAGS    := 
    # Use pkg-config if available, otherwise fallback to standard flags
    LIBS       := -lglfw -lGL -lm
    EXE_EXT    := 
    RM         := rm -rf
endif

FULL_TARGET := $(TARGET)$(EXE_EXT)

# --- Rules ---

# 1. The primary target: Deletes everything first, then builds
all: clean $(FULL_TARGET)

# 2. Linking
$(FULL_TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LIBS) -o $@

# 3. Compiling (with directory creation)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 4. Cleaning
clean:
	$(RM) $(BUILD_DIR) $(FULL_TARGET)

# Header dependencies
-include $(DEP)

.PHONY: all clean