# ============================================================
# Makefile – CPU Scheduling Simulator (SJF & Round Robin)
#
# Targets:
#   make        – build both executables
#   make sjf    – build SJF only
#   make rr     – build RR only
#   make run    – build and run both simulators
#   make clean  – remove executables
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g

.PHONY: all run clean

# Default target: build both
all: sjf rr

# Build SJF executable
sjf: sjf.c
	$(CC) $(CFLAGS) -o sjf sjf.c
	@echo "Built: sjf"

# Build Round Robin executable
rr: rr.c
	$(CC) $(CFLAGS) -o rr rr.c
	@echo "Built: rr"

# Build and run both
run: all
	@echo "\n--- Running SJF ---"
	./sjf
	@echo "\n--- Running Round Robin ---"
	./rr

# Remove compiled binaries
clean:
	rm -f sjf rr
	@echo "Cleaned up executables."
