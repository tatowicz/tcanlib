# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Object files
OBJS = ctp.o test_ctp.o

# Target executable
TARGET = ctp_test.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

ctp.o: ctp.c ctp.h
	$(CC) $(CFLAGS) -c ctp.c

test_ctp.o: test_ctp.c ctp.h
	$(CC) $(CFLAGS) -c test_ctp.c

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
