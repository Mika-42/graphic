PROJECT_NAME := app
CC := g++

SRC_DIR := srcs
BUILD_DIR := build

# Modules (ordre IMPORTANT)
MODULES := \
	$(SRC_DIR)/sanitize.cppm \
	$(SRC_DIR)/mouse.cppm \
	$(SRC_DIR)/context.cppm \
	$(SRC_DIR)/window.cppm \
	$(SRC_DIR)/uniform.cppm \
	$(SRC_DIR)/shader.cppm \
	$(SRC_DIR)/renderer.cppm

CPPS := $(shell find $(SRC_DIR) -name "*.cpp")
CS := $(shell find $(SRC_DIR) -name "*.c")

# Objets
MODULE_OBJS := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(MODULES:.cppm=.o))
CPP_OBJS := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(CPPS:.cpp=.o))
C_OBJS := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(CS:.c=.o))

OBJS := $(MODULE_OBJS) $(CPP_OBJS) $(C_OBJS)

FLAGS := -std=c++20 -fmodules -Wall -Wextra -Werror -Wpedantic -DDEBUG
LINK_FLAGS := -lglfw -lGL -lfreetype
INCLUDES := -I$(SRC_DIR) -Iglad -I/usr/include/freetype2

.PHONY: all clean

all: $(BUILD_DIR) $(OBJS)
	$(CC) $(OBJS) -o $(BUILD_DIR)/$(PROJECT_NAME) $(LINK_FLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 🔥 Compilation des modules (dans le bon ordre)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cppm
	@mkdir -p $(dir $@)
	$(CC) $< $(INCLUDES) $(FLAGS) -c -o $@

# 🔧 .cpp
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $< $(INCLUDES) $(FLAGS) -c -o $@

# 🔧 .c
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	gcc $< -Iglad -c -o $@

clean:
	rm -rf $(BUILD_DIR) gcm.cache
