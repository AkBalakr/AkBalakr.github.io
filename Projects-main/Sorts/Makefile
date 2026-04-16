CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic

sorting:
	make all
all: sorting.o insert.o heap.o stats.o batcher.o quick.o
	$(CC) -o sorting sorting.o insert.o heap.o quick.o batcher.o stats.o

sorting.o:
	$(CC) $(CFLAGS) -c sorting.c 

insert.o:
	$(CC) $(CFLAGS) -c insert.c

heap.o:
	$(CC) $(CFLAGS) -c heap.c

quick.o:
	$(CC) $(CFLAGS) -c quick.c

batcher.o:
	$(CC) $(CFLAGS) -c batcher.c

stats.o:
	$(CC) $(CFLAGS) -c stats.c
format:
	clang-format -i --style=file *.h *.c

clean:
	rm sorting *.o
