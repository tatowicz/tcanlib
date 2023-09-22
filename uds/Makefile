# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Object files
OBJS = uds.o test_uds.o

# Target executable
TARGET = uds_test.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -l libctp.a

uds.o: uds.c uds.h 
	$(CC) $(CFLAGS) -c uds.c -I ../ctp

test_uds.o: test_uds.c uds.h
	$(CC) $(CFLAGS) -c test_uds.c

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)