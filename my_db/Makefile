CC=cc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -Wvla -Werror
DFLAGS=-g -fsanitize=address
TFLAGS=-no-pie -fno-pie

SRC=src/main.c src/utils.c src/mapper.c src/symbols.c src/break.c
BIN=my_db
SRCEX=test/example.c
BINEX=test/example
TRASH=*.txt *.o

all:
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

example:
	$(CC) $(CFLAGS) $(TFLAGS) $(SRCEX) -o $(BINEX) 

debug:
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC) -o $(BIN)

clean:
	$(RM) $(BIN) $(TRASH)

