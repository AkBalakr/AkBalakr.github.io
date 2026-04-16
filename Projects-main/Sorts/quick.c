#include "quick.h"

#include "stats.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
uint32_t partition(
    Stats *stats, uint32_t *A, uint32_t lo, uint32_t hi) { //Find a good spot to partition the array
    uint32_t i = lo - 1; //lo 1 refers to first elements of array
    for (uint32_t j = lo; j < hi; j++) {
        if (cmp(stats, A[j - 1], A[hi - 1])
            == -1) { //if current element is less than last element swap
            i += 1; //i counts the number of elements less than the last element
            swap(stats, &A[i - 1], &A[j - 1]);
        }
    }
    swap(stats, &A[i],
        &A[hi
            - 1]); //By swapping the last number with the index of i the last number will be placed in its correct location

    return i + 1; //undo the subtract 1 from earlier
}
void quick_sorter(Stats *stats, uint32_t *A, uint32_t lo, uint32_t hi) {
    if (lo < hi) { //if lo is less than high then the subset of the array can still be divided
        uint32_t p = partition(stats, A, lo, hi); //Find the partitions
        quick_sorter(stats, A, lo,
            p - 1); //Lower bound of the partition excluding the element at the partition
        quick_sorter(stats, A, p + 1,
            hi); //Upper bound of the partitions excudign the element at the partition
    }
}
void quick_sort(Stats *stats, uint32_t *A, uint32_t n) {
    quick_sorter(stats, A, 1, n); //Call the quick sort
    printf("Quick Sort, %d elements, %" PRIu64 " moves, %" PRIu64 " compares\n", n, stats->moves,
        stats->compares);
}
