#include "batcher.h"
#include "heap.h"
#include "insert.h"
#include "quick.h"
#include "set.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void print_help() {
    char c;
    FILE *help_file = fopen("help", "r");

    c = fgetc(help_file);
    while (c != EOF) {
        printf("%c", c);
        c = fgetc(help_file);
    }

    fclose(help_file);
}
void dupe_array(uint32_t *A, uint32_t *B, uint32_t elements) {
    for (uint32_t e = 0; e < elements; e++) {
        B[e] = A[e];
    }
}

uint32_t f(uint32_t n) {
    return (n & 0x1) ? 3 * n + 1 : n / 2;
}

int main(int argc, char **argv) {
    //char c;
    //FILE *help_file = fopen("help", "r");

    int ch;
    uint32_t elements = 100;
    uint32_t e_print = elements; //default elements

    uint32_t r_seed = 13371453; //default random seed is 1000
    Stats *stats = (Stats *) malloc(sizeof(Stats)); //Allocate Memory
    reset(stats);
    Set options = empty_set();

    while ((ch = getopt(argc, argv, "aihqbp:r:n:H")) != -1) {

        switch (ch) {
        case 'a':
            options = insert_set(1, options);
            options = insert_set(2, options);
            options = insert_set(3, options);
            options = insert_set(4, options);
            break;
        case 'i': //insert = true;
            options = insert_set(1, options);
            break;
        case 'h': //heap = true;
            options = insert_set(2, options);
            break;
        case 'q': //quick = true;
            options = insert_set(3, options);
            break;
        case 'b': //batcher = true;
            options = insert_set(4, options);
            break;
        case 'r':
            r_seed = strtoimax(optarg, NULL, 10);
            //printf("optarg: %s\n", optarg);
            //printf("r_seed: %d\n", r_seed);

            break;
        case 'n':
            elements = strtoimax(optarg, NULL, 10);
            if (elements < e_print) {
                e_print = elements;
            }
            //printf("elements: %d\n", elements);
            break;
        case 'p': e_print = strtoimax(optarg, NULL, 10); break;
        case 'H': print_help(); break;

        default:
            printf("Select at least one sort to perform.\n");
            print_help();
            break;
        }
    }

    if (argc == 1) {
        printf("Select at least one sort to perform.\n");
        print_help();
    }
    uint32_t *A = (uint32_t *) calloc(elements, sizeof(uint32_t));
    srandom(r_seed);
    for (uint32_t e = 0; e < elements; e++) {
        A[e] = rand() & 0x3fffffff;
    }

    uint32_t *B = (uint32_t *) calloc(elements, sizeof(uint32_t));

    dupe_array(A, B, elements);
    if (member_set(1, options)) {
        insertion_sort(stats, B, elements);
        output_array(B, e_print);
        reset(stats);
    }

    dupe_array(A, B, elements);
    if (member_set(2, options)) {
        batcher_sort(stats, B, elements);
        output_array(B, e_print);
        reset(stats);
    }

    dupe_array(A, B, elements);
    if (member_set(3, options)) {
        heap_sort(stats, B, elements);
        output_array(B, e_print);
        reset(stats);
    }
    dupe_array(A, B, elements);
    if (member_set(4, options)) {
        quick_sort(stats, B, elements);
        output_array(B, e_print);
        reset(stats);
    }

    free(stats);
    free(A);
    free(B);

    return 0;
}
