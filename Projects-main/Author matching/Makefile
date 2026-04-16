CC = clang
CFLAGS = -Wall -Werror -Wextra -Wpedantic -g
#CFLAGS = -Wall -Wextra -Wpedantic

all:
	make identify

identify: pq.o ht.o node.o speck.o parser.o text.o bv.o bf.o
	$(CC) -o identify identify.c pq.o ht.o node.o speck.o parser.o text.o bv.o bf.o -lm

pq.o: pq.c
	$(CC) $(CFLAGS) -c pq.c -g

ht.o: ht.c
	$(CC) $(CFLAGS) -c ht.c -g 

node.o: node.c
	$(CC) $(CFLAGS) -c node.c -g 

speck.o: speck.c
	$(CC) $(CFLAGS) -c speck.c -g 

parser.o: parser.c
	$(CC) $(CFLAGS) -c parser.c -g 

text.o: text.c
	$(CC) $(CFLAGS) -c text.c -g

bv.o: bv.c
	$(CC) $(CFLAGS) -c bv.c -g

bf.o: bf.c
	$(CC) $(CFLAGS) -c bf.c -g

format:
	clang-format -i --style=file *.h *.c identify.c

clean:
	rm -f identify *.o

