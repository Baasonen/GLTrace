CC := gcc
CXX := g++
TARGET := glt

SRC_C   := $(wildcard src/*.c)
SRC_CXX := $(wildcard src/*.cpp)

IMGUI_DIR := external/imgui
IMGUI_SRC := \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_glfw.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

OBJ := $(patsubst src/%.c,build/%.o,$(SRC_C)) \
       $(patsubst src/%.cpp,build/%.o,$(SRC_CXX)) \
       $(patsubst $(IMGUI_DIR)/%.cpp,build/imgui/%.o,$(IMGUI_SRC))

ifeq ($(OS),Windows_NT)
    GLFW_INC ?= C:/libs/glfw/include
    GLFW_LIB ?= C:/libs/glfw/lib

    INCLUDES := -Iinclude -I$(GLFW_INC) -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
    CFLAGS   := $(INCLUDES) -Wall -O2
    CXXFLAGS := $(INCLUDES) -Wall -O2 -std=c++17
    LDFLAGS  := -L$(GLFW_LIB)
    LIBS     := -lglfw3 -lopengl32 -lgdi32
    TARGET   := $(TARGET).exe
else
    INCLUDES := -Iinclude -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
    CFLAGS   := $(INCLUDES) -Wall -O2
    CXXFLAGS := $(INCLUDES) -Wall -O2 -std=c++17
    LDFLAGS  :=
    LIBS     := -lglfw -lGL -lm
endif

all: clean $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $^ $(LDFLAGS) $(LIBS) -o $@

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/imgui/%.o: $(IMGUI_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)

.PHONY: all clean