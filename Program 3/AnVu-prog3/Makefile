# Makefile for Program 3 Memory Management

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
TARGET = prog3
SRC = prog3.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Provided test files
run1: $(TARGET)
	./$(TARGET) < prog3.input1

run2: $(TARGET)
	./$(TARGET) < prog3.input2

clean:
	rm -f $(TARGET)
