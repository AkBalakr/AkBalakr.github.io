CC = clang
CFLAGS = -Wall -Wextra -Wpedantic -Werror

all:
	make life

life:  universe.o
	$(CC) $(CFLAGS) -o life life.c universe.o -lncurses


universe.o:
	$(CC) $(CFLAGS) -c universe.c

format:
	clang-format -i --style=file *.h *.c

clean:
	rm life *.o


