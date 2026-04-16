#include "batcher.h"

#include "stats.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
void comparator(Stats *stats, uint32_t *A, int32_t x, uint32_t y) {
    if (cmp(stats, A[x], A[y]) == 1) { // A[x] > A[y] then A[x] swaps with A[y]
        swap(stats, &A[x], &A[y]);
    }
}
void batcher_sort(Stats *stats, uint32_t *A, uint32_t n) {

    if (n == 0) { //if no Array you can't sort it
        return;
    }

    uint32_t t = 0;
    for (uint32_t b = n; b > 0;
         b >>= 1) { //keep dividing by 2 until it becomes 0 to find bit length
        t++;
    }
    uint32_t p = 1 << (t - 1); //Greatest gap that is some 2^x

    while (p > 0) { //While gap is greater than 0
        uint32_t q = 1 << (t - 1);
        uint32_t r = 0;
        uint32_t d = p;

        while (d > 0) {
            for (uint32_t i = 0; i < n - d; i++) {
                if ((i & p) == r) { //Determines if able to be compared with gap p
                    comparator(stats, A, i, i + d);
                }
            }
            d = q - p;
            q >>= 1;
            r = p;
        }
        p >>= 1; //Divide Gap by 2
    }
    printf("Batcher Sort, %d elements, %" PRIu64 " moves, %" PRIu64 " compares\n", n, stats->moves,
        stats->compares);
}
