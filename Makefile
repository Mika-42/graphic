PROJECT_NAME := app
CC := g++
FLAGS := -lglfw -lGL
SRCS := srcs/context.cppm srcs/window.cppm srcs/draw.cppm srcs/glad.c srcs/main.cpp

debug:
	$(CC) $(SRCS) -Isrcs -Iglad -IKHR -o $(PROJECT_NAME) $(FLAGS) -DNDEBUG -Og -fmodules -Wall -Wextra -Werror -Wpedantic
