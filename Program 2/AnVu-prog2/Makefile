# Makefile for Program 2 - Scheduling Simulation

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
TARGET = prog2
SRC = prog2.c

# Default target (build the program)
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Run the program with optional input file
run: $(TARGET)
	./$(TARGET) < prog2.input

# Remove the compiled executable
clean:
	rm -f $(TARGET)
