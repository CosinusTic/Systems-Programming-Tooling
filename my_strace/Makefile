CC=cc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -Wvla -Werror
DFLAGS=-g -fsanitize=address

SRC=src/main.c src/syscalls.c
BIN=my_strace
TRASH=*.txt *.o
all:
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

debug:
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC) -o $(BIN)

clean:
	$(RM) $(BIN) $(TRASH)

