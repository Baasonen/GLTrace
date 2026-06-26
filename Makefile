CC := gcc
TARGET := glt
SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))

ifeq ($(OS),Windows_NT)
    GLFW_INC ?= C:/libs/glfw/include
    GLFW_LIB ?= C:/libs/glfw/lib

    CFLAGS := -Iinclude -I$(GLFW_INC) -Wall -O2
    LDFLAGS := -L$(GLFW_LIB)
    LIBS := -lglfw3 -lopengl32 -lgdi32
    TARGET := $(TARGET).exe
else
    CFLAGS := -Iinclude -Wall -O2
    LDFLAGS :=
    LIBS := -lglfw -lGL -lm
endif

all: clean $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)

.PHONY: all clean