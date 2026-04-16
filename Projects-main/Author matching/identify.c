/*
	Contains main() and implementation of the program
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h> //getopt
#include "metric.h"
#include "parser.h"
#include "text.h"
#include "pq.h"

void usage(void) {
    fprintf(stderr, "usage: identity [-d:n:k:l:emch]\n");
    fprintf(stderr,
        "-h : \n -d database Database of authors and texts [default: lib.db]\n -k matches "
        "Set number of top matches to display [default: 5] \n -n noisefile Set file of "
        "words to ignore [default: noise.txt] \n -l limit Set noise word limit [default: "
        "100] \n -e Set distance metric as Euclidean distance [default] \n -m Set distance "
        "metric as Manhattan distance \n -c Set distance metric as cosine distance \n -v "
        "Enable verbose printing of program run \n -h Display program help and usage");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    //printf("Identify\n");
    FILE *anon = stdin; //Anonymous text needing to be identified
    FILE *ref;
    FILE *database;
    FILE *noise;

    char *noise_filename = "noise.txt";
    char *anon_filename;
    char *database_filename = "lib.db";
    char *ref_filename;

    noiselimit = 100; //Number of noise words to filer out DEFAULT: 100
    uint32_t matches = 10;

    Metric metric = EUCLIDEAN; //Default is Euclidean
    bool Euclidean = true;
    bool Manhattan = false;
    bool Cosine = false;
    //printf("Getting Options\n");
    char ch; //Chosen options
    while ((ch = getopt(argc, argv, "d:n:k:l:emch")) != -1) {
        switch (ch) {
        case 'd': //Path to database of authors and texts
            database_filename = optarg;
            break;
        case 'n': //Path to file of noise words to filter out
            noise_filename = optarg;
            break;
        case 'k': //Number of Matches outputted
            matches = strtoimax(optarg, NULL, 10);
            break;
        case 'l': //Number of noise words to Filter out
            noiselimit = strtoimax(optarg, NULL, 10);
            break;
        case 'e': //Euclidean Distance
            metric = EUCLIDEAN;
            //Euclidean = true; Manhattan = false; Cosine = false;
            break;
        case 'm': //Manhattan Distance
            metric = MANHATTAN;
            //Euclidean = false; Manhattan = true; Cosine = false;
            break;
        case 'c': //Cosine Distance
            metric = COSINE;
            //Euclidean = false; Manhattan = false; Cosine = true;
            break;
        case 'h': //Help/Usage
            usage();
            break;
        }
    }
    //printf("Opening Files\n");
    //Noise Text
    noise = fopen(noise_filename, "r");
    if (noise == NULL) {
        fprintf(stderr, "%s is NULL\n", noise_filename);
    }

    Text *noise_text = text_create(noise, NULL); //Make noise Text
    fclose(noise); //Noise file no longer needed
    //printf("<-----DONE WITH NOISE---->\n\n");
    database = fopen(database_filename, "r");
    //printf("open database\n");

    //DataBase
    //int entries; //Number of Author/Text pairs in Database
    char entries[32];
    fgets(entries, 32, database);
    uint32_t pairs = strtoimax(entries, NULL, 10);
    //printf("Author/Text Pairs: %"PRIu32"\n", pairs);

    uint32_t a_size = 256; //fgets requires a size and wants a character array
    char author[a_size], author_text[a_size]; //Author, Name of txt by author
    //Make ADT for text Anonymous Files

    //MAKE A CLIPPER FUNCTION
    /*
	fgets(author, a_size, database);
	fgets(author_text, a_size, database);
	for(uint32_t nl = a_size; nl > 0; nl--) { //Removing the '\n' not sure it matters EDIT: IT DOES MATTER
		if (author[nl-1] == '\n') author[nl-1] = '\0';
		if (author_text[nl-1] == '\n') author_text[nl-1] = '\0';
	}
	

	printf("open %s by %s\n", author_text, author); //First Pair is Anon
	anon = fopen(author_text, "r");
	*/

    //anon = fopen(anon_filename, "r");
    if (anon == NULL) {
        fprintf(stderr, "%s is NULL\n", author_text);
    } else {
        printf("\n");
    }

    regex_t re_word;
    regcomp(&re_word, "[a-zA-Z]+['-]?[a-zA-Z]+", REG_EXTENDED);

    char *word;
    // printf("Next_word\n");
    word = next_word(anon, &re_word);
    //printf("Word %s\n", word);

    double distance;
    Text *anon_text = text_create(anon, noise_text);
    fclose(anon);
    //printf("close anon file\n");
    //printf("<-----DONE WITH ANON----->\n\n");

    //Loop for every Author/Text pair
    //Make ADT Text for known Author Files
    PriorityQueue *pq = pq_create(pairs);

    for (uint32_t e = 0; e < pairs; e++) {
        fgets(author, a_size, database);
        fgets(author_text, a_size, database);
        for (uint32_t nl = a_size; nl > 0; nl--) { //Removing the '\n' not sure it matters
            if (author[nl - 1] == '\n')
                author[nl - 1] = '\0';
            if (author_text[nl - 1] == '\n')
                author_text[nl - 1] = '\0';
        }

        //printf("open reffile\n"); //Rest are Known
        ref = fopen(author_text, "r");
        if (ref == NULL) {
            fprintf(stderr, "%s is NULL\n", author_text);
        }
        //printf("open %s by %s\n", author_text, author); //First Pair is Anon
        Text *ref_text = text_create(ref, noise_text);
        //printf("<-----DONE WITH REF------>\n\n");

        //Find the distance between the Anonymous and Known
        //Important that it is anon then ref because ref nodes get deleted
        //distance = text_dist(anon_text, ref_text, NULL); //Using NULL for metric because I do not know how to use it yet
        distance = text_dist(anon_text, ref_text, metric);

        //printf("\nMINI OUTPUT: %s [%f]\n", author, distance);
        enqueue(pq, author, distance);
        text_delete(&ref_text); //Repeats after every File File comparison is done
    }

    printf("Top %" PRIu32 ", metric: %s, noise limit: %" PRIu32 "\n", matches, metric_names[metric],
        noiselimit);
    char *deq_author;
    for (uint32_t k = 0; k < matches; k++) {
        printf("\n %" PRIu32 ") ", k + 1);
        dequeue(pq, &deq_author, &distance);
        printf("\n");
    }
    //printf("End of Main functionality\n");

    fclose(ref);
    fclose(database);

    //Free anon and noise text ADTs
    text_delete(&noise_text);
    text_delete(&anon_text);
    //printf("noise limit %"PRIu32"\n", noiselimit);
    //printf("End of Identify Main()\n");

    return 0;
}
