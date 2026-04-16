#include "stats.h"

#include <inttypes.h>
#include <stdio.h>

int cmp(Stats *stats, uint32_t x, uint32_t y) {
    stats->compares += 1;
    if (x < y) {
        return -1;
    } else if (x > y) {
        return 1;
    } else {
        return 0;
    }
}

uint32_t move(Stats *stats, uint32_t x) {
    stats->moves += 1;
    return x;
}

void swap(Stats *stats, uint32_t *x, uint32_t *y) {
    stats->moves += 3;
    uint32_t t = *x;
    *x = *y;
    *y = t;
}

void reset(Stats *stats) {
    stats->moves = stats->compares = 0;
}

void print_array(uint32_t *A, uint32_t n) {
    for (uint32_t i = 1; i <= n; i++) {
        printf("%5" PRIu32, A[i - 1]);
    }
    printf("\n");
}

void output_array(uint32_t *A, uint32_t n) {
    if (n == 0) {
        return;
    }
    uint32_t i = 0;
    for (i = 0; i < n; i++) {
        if (i != 0 && i % 5 == 0) {
            printf("\n");
        }
        printf("%13" PRIu32, A[i]);
    }
    printf("\n");
}
