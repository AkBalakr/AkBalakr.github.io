# Assignment 6: Huffman Coding
## Description:
	Huffman is meant to reduce sizes of files. This program works with specifically with text files and should work at any inputted file length. 
	Encrypt: Finds a more efficient code for each character and outputs an encode with a smaller number of bytes
	Decrypt: Decrypts an inputted file and expands the bytes into chosen output. 

## Build:
	make
	make all
	make encode
	make decode

## Options:
	-h             Program usage and help.
	-v             Print compression statistics.
	-i infile      Input file to compress.
	-o outfile     Output of compressed data.

## Run:
	./encode [-h] [-i infile] [-o outfile]
	./decode [-h] [-i infile] [-o outfile]

## Errors:
	Warnings: Some type conversion errors exist but is vital to functionality of the program exist that I was not able to fix yet.
	Last Character: The decrypt converts everything but the last character is an issue. However this error is not replicated in all input files.





