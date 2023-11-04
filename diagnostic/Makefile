CC = gcc
CFLAGS = -Wall -g -Wextra -std=c99
SERVER_BIN = test_server
SERVER_SRC = test_server.c server.c ../ctp/ctp.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)

all: $(SERVER_BIN) 

$(SERVER_BIN): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(SERVER_BIN) $(SERVER_OBJ)


#$(CLIENT_BIN): $(CLIENT_OBJ) 
#	$(CC) $(CFLAGS) -o $@ $^

#%.o: %.c
#	$(CC) $(CFLAGS) -c $< -o $@ -I ../ctp

server.o: server.c server.h
	$(CC) $(CFLAGS) -c server.c -I ../ctp

test_server.o: test_server.c server.h 
	$(CC) $(CFLAGS) -c test_server.c -I ../ctp


test: $(SERVER_BIN)
	./$(SERVER_BIN)

clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) $(SERVER_BIN) $(CLIENT_BIN) 

.PHONY: all clean