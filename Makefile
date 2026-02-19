# Makefile for myshell

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = myshell
SRC = myshell.c
TEST_SCRIPT = run_tests.sh

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

test: $(TARGET)
	bash ./$(TEST_SCRIPT)

.PHONY: all clean test

