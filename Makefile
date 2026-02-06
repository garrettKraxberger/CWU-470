CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = hello
SRCS = Hello.c

.PHONY: all clean run lab2 run-lab2

all: $(TARGET) lab2

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

lab2: Lab2.c
	$(CC) $(CFLAGS) -o lab2 Lab2.c

run: all
	./$(TARGET)

run-lab2: lab2
	./lab2

clean:
	rm -f $(TARGET) lab2 *.o source_folder/* destin_folder/*
