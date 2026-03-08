CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -g -O0
CPPFLAGS = -Iinclude

BIN = bin/mkd
TEST_BIN = bin/test_smoke

SRC = src/main.c src/core/kernel.c src/core/log.c src/core/scheduler.c src/core/task.c
TEST_SRC = tests/test_smoke.c src/core/kernel.c src/core/log.c src/core/scheduler.c src/core/task.c

all: $(BIN)

$(BIN): $(SRC) include/kernel/kernel.h
	mkdir -p bin
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $(BIN)

$(TEST_BIN): $(TEST_SRC) include/kernel/kernel.h
	mkdir -p bin
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_SRC) -o $(TEST_BIN)

run: $(BIN)
	./$(BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

clean: 
	rm -rf bin build

.PHONY: all run clean test


