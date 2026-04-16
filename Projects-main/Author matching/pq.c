#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "pq.h"
#include "node.h"

typedef struct AuthorDistance AuthorDistance;

struct AuthorDistance {
    char *author;
    float distance;
};

struct PriorityQueue {
    uint64_t capacity;
    uint32_t size;
    AuthorDistance **ad; //Array of Author/Distance pairs
};

PriorityQueue *pq_create(uint32_t capacity) {
    PriorityQueue *pq = (PriorityQueue *) malloc(sizeof(PriorityQueue));
    pq->capacity = capacity;
    pq->size = 0;
    pq->ad = (AuthorDistance **) calloc(capacity, sizeof(AuthorDistance *));
    return pq;
}

AuthorDistance *ad_create(char *author, float distance) {
    AuthorDistance *ad = (AuthorDistance *) malloc(sizeof(AuthorDistance));
    ad->author = strdup(author);
    ad->distance = distance;
    return ad;
}

void pq_delete(PriorityQueue **q) {
    free((*q)->ad);
    (*q)->ad = NULL;
    free(*q);
}

bool pq_empty(PriorityQueue *q) {
    return q->size == 0;
}

bool pq_full(PriorityQueue *q) {
    return q->size == q->capacity;
}

uint32_t pq_size(PriorityQueue *q) {
    return q->size;
}

bool enqueue(PriorityQueue *q, char *author, double dist) {
    if (pq_full(q))
        return false; //Cannot enqueue if full
    q->ad[pq_size(q)] = ad_create(author, dist);
    q->size++;

    //Insertion Sort
    //Trying to make min queue, least distance goes to highest index
    uint32_t size = pq_size(q);
    int j = 0;
    //printf("Enq. Author %s, %f\n", author, dist);
    AuthorDistance *tmp_ad = NULL;
    for (uint32_t i = 1; i < size; i++) {
        tmp_ad = q->ad[i]; //tmp ad = current ad
        j = i - 1;

        while (j >= 0 && q->ad[j]->distance < tmp_ad->distance) { //If previous is less than swap
            q->ad[j + 1] = q->ad[j];
            j = j - 1;
        }
        q->ad[j + 1] = tmp_ad;
    }
    return true;
}

bool dequeue(PriorityQueue *q, char **author, double *dist) {
    if (pq_empty(q))
        return false; //Cannot dequeue if empty
    q->size--;

    *author = q->ad[pq_size(q)]->author;
    *dist = q->ad[pq_size(q)]->distance;
    printf("%s [%.15f]\n", *author, *dist);
    return true;
}

void pq_print(PriorityQueue *q) {
    printf("PriorityQueue\n");
    for (uint32_t i = 0; i < pq_size(q); i++) {
        printf("	%s %f\n", q->ad[i]->author, q->ad[i]->distance);
    }
}
