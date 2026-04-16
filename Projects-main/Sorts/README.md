# Assignment 3: Sorting: Putting your affairs in order

## Description:
	This program does Insertion, Batcher, Heap, and Quick Sort. It generates random array
	of default 100 elements and outputs the sorted value.
	OPTIONS
   -H              Display program help and usage.
   -a              Enable all sorts.
   -i              Enable Insertion Sort.
   -b              Enable Batcher Sort.
   -h              Enable Heap Sort.
   -q              Enable Quick Sort.
   -n length       Specify number of array elements (default: 100).
   -p elements     Specify number of elements to print (default: 100).
   -r seed         Specify random seed (default: 13371453).

## Build: any of the following
	make
	make all
	make sorting
## Run:
	./sorting [-Haibhqn:p:r:] [-n length] [-p elements] [-r seed]

## Clean:
	make clean

## Format: clang-format
	make format

## Files:
	heap, insert, quick, batcher.{c, h}
	stat.{c, h}
	set.h
	sorting.c
	help //file containing output for -H
	Makefile
	README.md
	WRITEUP.pdf
	DESIGN.pdf
	plot.sh
