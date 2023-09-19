# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Object files
OBJS = sha256.o test_sha256.o

# Target executable
TARGET = sha256.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

sha256.o: sha256.c sha256.h
	$(CC) $(CFLAGS) -c sha256.c

test_sha256.o: test_sha256.c sha256.h
	$(CC) $(CFLAGS) -c test_sha256.c

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)