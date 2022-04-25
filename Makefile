CFLAGS   = -Wall -std=gnu99
INCLUDES = -I./inc

CLIENT_SRCS = src/defines.c src/err_exit.c src/shared_memory.c src/semaphore.c src/fifo.c src/client_0.c src/client_functions.c
SERVER_SRCS = src/defines.c src/err_exit.c src/shared_memory.c src/semaphore.c src/fifo.c src/server.c src/server_functions.c

SERVER_OBJS = $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

all: server client_0

client_0: $(CLIENT_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@

server: $(SERVER_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@

.c.o:
	@echo "Compiling: "$<
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -vf ${CLIENT_OBJS}
	@rm -vf ${SERVER_OBJS}
	@rm -vf client_0
	@rm -vf server
	@ipcrm -a
	@echo "Removed object files and executables..."

.PHONY: run clean
