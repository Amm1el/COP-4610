SRC := src
OBJ := obj
BIN := bin
EXECUTABLE := shell

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
INCS := -Iinclude/
EXEC := $(BIN)/$(EXECUTABLE)

CC := gcc
CFLAGS := -g -Wall -std=c99 -D_POSIX_C_SOURCE=200809L $(INCS)
LDFLAGS :=

all: $(EXEC)

$(EXEC): $(OBJS) | $(BIN)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ):
	mkdir -p $(OBJ)

$(BIN):
	mkdir -p $(BIN)

run: $(EXEC)
	$(EXEC)

clean:
	rm -rf $(OBJ) $(EXEC)

.PHONY: all run clean
