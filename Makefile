PROJECT_NAME := app
CC := g++
FLAGS := -lglfw -lGL
SRCS := srcs/context.cppm srcs/window.cppm srcs/draw.cppm srcs/glad.c srcs/main.cpp
BUILD_DIR := build

.PHONY: $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

debug:
	$(CC) $(SRCS) -Isrcs -Iglad -IKHR -o $(BUILD_DIR)/$(PROJECT_NAME) $(FLAGS) -DNDEBUG -Og -fmodules -Wall -Wextra -Werror -Wpedantic

clean:
	rm -rf $(BUILD_DIR)
