CC ?= gcc

TARGET ?= glt
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

GLFW_INC ?= C:/libs/glfw/include
GLFW_LIB ?= C:/libs/glfw/lib

CFLAGS := -I$(INC_DIR) -I$(GLFW_INC) -Wall -MMD -MP -O2
LDFLAGS := -L$(GLFW_LIB)
LIBS := -lglfw3 -lopengl32 -lgdi32

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

-include $(DEP)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean
