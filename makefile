NAME = server

TARGET_DIR = bin
OBJ_DIR = obj

SRC = $(wildcard src/*.c) $(wildcard src/*/*.c)
OBJ = $(addprefix $(OBJ_DIR)/, $(patsubst %.c, %.o, $(SRC)))
HEADERS = $(wildcard include/*.h) $(wildcard include/*/*.h)
TARGET = $(TARGET_DIR)/$(NAME)

NECESSARY_DIRS = $(dir $(OBJ) $(TARGET))

CC = gcc
CFLAGS = -c -Iinclude

$(OBJ_DIR)/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(OBJ) $(MAIN_OBJ)
	$(CC) $^ -o $@

$(sort $(NECESSARY_DIRS)):
	mkdir -p $@

.PHONY: compile run clean dirs

clean:
	rm -rf obj bin

compile: $(NECESSARY_DIRS) $(TARGET)