CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
SERVER_BIN = server
CLIENT_BIN = client 
SERVER_SRC = server.c ../ctp/ctp.c
CLIENT_SRC = client.c ../ctp/ctp.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o) 

all: $(SERVER_BIN) $(CLIENT_BIN) 

$(SERVER_BIN): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_BIN): $(CLIENT_OBJ) 
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -I ../ctp

clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) $(SERVER_BIN) $(CLIENT_BIN)  # Added new binary and object file

.PHONY: all clean