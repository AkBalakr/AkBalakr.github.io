CC = clang
CFLAGS = -Wall -Werror -Wextra -Wpedantic
#CFLAGS = -Wall  -Wextra -Wpedantic

all: 
	make encode 
	make decode


decode: decode.o pq.o node.o stack.o huffman.o code.o io.o
	$(CC) -o decode decode.c pq.o node.o stack.o huffman.o code.o io.c

encode: encode.o pq.o node.o stack.o huffman.o code.o io.o
	$(CC) -o encode encode.c pq.o node.o stack.o huffman.o code.o io.c

node.o: node.c
	$(CC) $(CFLAGS) -c node.c

stack.o: stack.c
	$(CC) $(CFLAGS) -c stack.c

pq.o: pq.c
	$(CC) $(CFLAGS) -c pq.c

huffman.o: huffman.c
	$(CC) $(CFLAGS) -c huffman.c

generator.o: generator.c
	$(CC) $(CFLAGS) -c generator.c

code.o: code.c
	$(CC) $(CFLAGS) -c code.c

io.o: io.c
	$(CC) $(CFLAGS) -c io.c

format:
	clang-format -i --style=file *.h *.c

clean:
	rm -f encode decode *.o