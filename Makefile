# ─────────────────────────────────────────────────────────────────
# Makefile – libFS Test Application
# Author : Garrett
# Target : Garrett_testFS  (linked with Claude_libFS)
# ─────────────────────────────────────────────────────────────────

CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g
TARGET  = Garrett_testFS

# Source & object files
SRCS    = Garrett_libFS.c Garrett_testFS.c
OBJS    = $(SRCS:.c=.o)

# ── Default target ──────────────────────────────────────────────
.PHONY: all
all: $(TARGET)
	@echo ""
	@echo "  Build complete → ./$(TARGET)"
	@echo "  Run with:  ./$(TARGET)"
	@echo ""

# ── Link ────────────────────────────────────────────────────────
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# ── Compile ─────────────────────────────────────────────────────
%.o: %.c Garrett_libFS.h
	$(CC) $(CFLAGS) -c -o $@ $<

# ── Run convenience target ──────────────────────────────────────
.PHONY: run
run: all
	./$(TARGET)

# ── Clean ───────────────────────────────────────────────────────
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET) Garrett_Introduction.txt
	@echo "  Cleaned build artifacts."

# ── Help ────────────────────────────────────────────────────────
.PHONY: help
help:
	@echo "  Targets:"
	@echo "    all    – build $(TARGET)  (default)"
	@echo "    run    – build and run the test application"
	@echo "    clean  – remove object files, binary, and generated .txt"
	@echo "    help   – show this message"
