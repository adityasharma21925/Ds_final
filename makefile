# Makefile for ASTP-MPI

CC = mpicc
PY_CFLAGS := $(shell python3-config --includes)
PY_LDFLAGS := $(shell python3-config --embed --ldflags 2>/dev/null || python3-config --ldflags)
CFLAGS = -Wall -Wextra -O2 -g -std=c11 -Iinclude $(PY_CFLAGS)
LDFLAGS = -lm $(PY_LDFLAGS)

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/astp

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "✓ Build complete: $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run16:
	mpirun -np 16 $(TARGET)

run50:
	mpirun -np 50 $(TARGET)

run100:
	mpirun -np 100 $(TARGET)

.PHONY: all clean run16 run50 run100