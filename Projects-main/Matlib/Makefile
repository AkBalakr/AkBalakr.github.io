CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic

.Phony: all

all: integrate #Phony Target
format:
	clang-format -i --style=file *.h *.c
test: function

function: function-test.o mathlib.o
	$(CC) -o function-test function-test.o mathlib.o -lm

integrate: integrate.o mathlib.o functions.o
	$(CC) -o integrate integrate.o mathlib.o functions.o

integrate.o: integrate.c
	$(CC) $(CFLAGS) -c integrate.c

mathlib.o: mathlib.c
	$(CC) $(CFLAGS) -c mathlib.c

funtions.o: functions.x
	$(CC) $(CFLAGS) -c functions.c

function-test.o: function-test.c
	$(CC) $(CFLAGS) -c function-test.c
clean:
	rm -f integrate *.o 
clean-test:
	rm -f function-test *.o
