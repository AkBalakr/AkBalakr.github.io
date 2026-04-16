
#include "insert.h"

#include "stats.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

void insertion_sort(Stats *stats, uint32_t *A, uint32_t n) {
    for (uint32_t i = 1; i < n; i++) { //Increment through the array
        uint32_t j = i; //j is the current index of the array
        uint32_t temp = A[i]; //storing value for current index of array
        move(stats, j); //temp = A[i] is a move since it involves a array element
        while (j > 0
               && cmp(stats, temp, A[j - 1])
                      == -1) { //Increment backwards starting at the current index of the array
            A[j] = A
                [j - 1]; //if the A[j] is less than a previous element A[j - 1] then A[j] needs to be placed into index of j - 1
            move(stats, j); //reassignment is a move
            j -= 1; //move backward 1 index
        }
        A[j]
            = temp; //j only goes backwards while temp = A[i] is less than [j - 1], therefore j would be the proper ordered index of A[i]
        move(stats, j); //A[j] = temp is reassignment and therefore a move
    }
    printf("Insert Sort, %d elements, %" PRIu64 " moves, %" PRIu64 " compares\n", n, stats->moves,
        stats->compares);
}
