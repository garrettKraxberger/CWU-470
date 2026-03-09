# Makefile – Paging Address Translation
# Author: Garrett

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
TARGET  = Garrett_paging

.PHONY: all clean

all: $(TARGET)
	@echo "Build complete → ./$(TARGET)"

$(TARGET): Garrett_paging.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
	@echo "Cleaned."
