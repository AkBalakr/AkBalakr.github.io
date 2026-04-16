#include "heap.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

uint32_t max_child(Stats *stats, uint32_t *A, uint32_t first, uint32_t last) {
    uint32_t left = 2 * first; //right child
    uint32_t right = left + 1; //left child

    if (right <= last
        && cmp(stats, A[right - 1], A[left - 1])
               == 1) { //if right child > left child return right index
        return right;
    }
    return left;
}

void fix_heap(Stats *stats, uint32_t *A, uint32_t first, uint32_t last) {
    uint32_t found = false; //Before you find something it is not found
    uint32_t mother = first; //first is the first parent
    uint32_t great = max_child(stats, A, mother, last); //Find the favorite child
    while (mother <= last / 2 && !found) { //Mother can only have children if it is <= last index/2
        if (cmp(stats, A[mother - 1], A[great - 1])
            == -1) { //If mother is less than the child then the child is mother
            swap(stats, &A[mother - 1], &A[great - 1]); //make child becomes the mother
            mother = great;
            great = max_child(stats, A, mother, last);
        } else {
            found = true;
        }
    }
}
void build_heap(Stats *stats, uint32_t *A, uint32_t first, uint32_t last) {
    for (uint32_t father = last / 2; father > first - 1; father--) {
        fix_heap(stats, A, father, last);
    }
}
void heap_sort(Stats *stats, uint32_t *A, uint32_t n) {
    uint32_t first = 1; //First element is counted at 1 in order to be able to do multiplication
    uint32_t last = n; //Last elenebt is the length of the array if count starts at 1
    build_heap(stats, A, first, last); //Build a Max heap, all fathers > than the children
    for (uint32_t leaf = last; leaf > first; leaf--) {
        swap(stats, &A[leaf - 1],
            &A[first
                - 1]); //Take the last index, and swap it with the ancestor which has the greatest value
        last--; //Now that the greatest value is at the last element now need to look for the next greatest
        fix_heap(
            stats, A, first, last); //Fix the Max Heap so max will be in the spot of the ancestor
    }
    printf("Heap Sort, %d elements, %" PRIu64 " moves, %" PRIu64 " compares\n", n, stats->moves,
        stats->compares);
}
