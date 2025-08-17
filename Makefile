APP     := http 
BUILD   := debug
CC      := gcc
STD     := c23

BIN_DIR := bin
OBJ_DIR := build/$(BUILD)

SRC      = http.c
OBJ     := $(SRC:%.c=$(OBJ_DIR)/%.o)

TARGET  := $(BIN_DIR)/$(APP)

WFLAGS  := -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wpointer-arith -Wcast-qual
CFLAGS  = $(WFLAGS) -std=$(STD)

ifeq ($(BUILD),debug)
  CFLAGS += -O0 -g
endif


.DEFAULT_GOAL := all

all: $(TARGET)

run: $(TARGET)
	./$(TARGET) $(ARGS)

debug:
	$(MAKE) BUILD=debug

clean:
	rm -rf build bin

.PHONY: all run debug clean


$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@
