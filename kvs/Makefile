# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Object files
OBJS = kvs.o test_kvs.o

# Target executable
TARGET = kvs_test.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

kvs.o: kvs.c kvs.h
	$(CC) $(CFLAGS) -c kvs.c

test_ctp.o: test_kvs.c kvs.h
	$(CC) $(CFLAGS) -c test_kvs.c

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
