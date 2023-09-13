# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Object files
OBJS = example.o

# Target executable
TARGET = example.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

example.o: example.c io.h
	$(CC) $(CFLAGS) -c example.c

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)