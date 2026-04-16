# Assignment 7: Author Identification
## Description:
	Author idenfitication uses a noise files (a file of common/overly used words), an anonymous file(file wanting to be compared) by stdin
	and a database file (a .db file that lists the paths of all the other to compare to the anonymous)
	Computes the distance between the inputted files and all other files given in the database. Then in stdout will give all the k closest files
	to the given file.
	The purpose of this would be to check the identity of an author.
## Build:
	make
	make all
	make identify

## Options:
   -d database     Database of authors and texts [default: lib.db]
   -k matches      Set number of top matches to display [default: 5]
   -n noisefile    Set file of words to ignore [default: noise.txt]
   -l limit        Set noise word limit [default: 100]
   -e              Set distance metric as Euclidean distance [default]
   -m              Set distance metric as Manhattan distance
   -c              Set distance metric as cosine distance
   -v              Enable verbose printing of program run
   -h              Display program help and usage

## Run:
	./identify<[anonymous.txt] [-d database] [-k matches]

## Errors:
	SEGFAULTS after running once. This might be due to memory leaks.
	The program never segfaults when run in the timeshare server. 
	The numbers of the 3 methods are off, most notably for Cosine which is always extremely close to 1. 
	Thought the order is usually correct.
	Takes a very very long time to run many files
