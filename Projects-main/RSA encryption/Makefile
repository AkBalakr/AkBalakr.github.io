
CC = clang
CFLAGS = -g -Wall -Wextra -Wpedantic -Werror
LFLAGS = $(shell pkg-config --libs gmp)

all:
	make keygen
	make encrypt
	make decrypt

decrypt: numtheory.o rsa.o randstate.o
	$(CC) $(CFLAGS) -o decrypt decrypt.c numtheory.o rsa.o randstate.o -lgmp

encrypt: numtheory.o rsa.o randstate.o
	$(CC) $(CFLAGS) -o encrypt encrypt.c numtheory.o rsa.o randstate.o -lgmp

keygen: numtheory.o rsa.o randstate.o
	$(CC) $(CFLAGS) -o keygen keygen.c numtheory.o rsa.o randstate.o -lgmp

rsa.o:
	$(CC) $(CFLAGS) -c rsa.c

numtheory.o:
	$(CC) $(CFLAGS) -c numtheory.c

randstate.o:
	$(CC) $(CFLAGS) -c randstate.c

pkg-config:
	sudo apt install pkg-config libgmp-dev

format:
	clang-format -i --style=file *.h *.c

clean:
	rm decrypt encrypt keygen *.o


