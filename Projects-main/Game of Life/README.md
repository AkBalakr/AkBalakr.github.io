# Assignment 4: The Game of Life

## Description:
	Play the game of life.
	Rules:
	- 3 neighbors: Dead to Life
	- 2 or 3 neighbors: Life to Life
	- Anything else: Life to Dead

## Run:
	./life [Options]

## Options:
	-t	Turns on torroidal
	-s	Does not animate
	-n:	Number of Generations
	-i:	Takes in input file to populat universe in {"$rol $col"} format, the first line must be the maximum dimensions of the universe
	-o:	Creates a file specifies and writes the output into it

## Build:
	- make
	- make all
	- make life



## Clean:
	- make clean


## Files:
	- universe.h, universe.c
	- life.c
	- Makefile
	- README.md
