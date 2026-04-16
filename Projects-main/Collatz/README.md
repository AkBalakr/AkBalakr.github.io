# Assignment 1: Getting Acquainted with UNIX and C

## Description
	This program revolves around the collatz sequence. It will compile and run collatz.c numerous times and compile 3 graphs. A dot plot of the sequence lengths at different starting values from 2 to 10000.
	Another dot plot for the maximum value for different starting values from 2 to 10000. 
	And the last is a histogram that represents the frequency of the lengths of the sequences from starting values 2 to 10000.

## Build
	chmod +x plot.sh	 makes plot.sh an excecutable
	
## Run
	./plot.sh

## Clean
	rm -rf tmp	 removes the .dat files
	rm maximum.pdf histogram.pdf sequence_lengths.pdf
