#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "pq.h"
#include "node.h"

struct PriorityQueue {
    uint64_t capacity; //Maximum
    uint32_t size; //Current Size

    Node **node; //Array/Queue of nodes
};

PriorityQueue *pq_create(uint32_t capacity) {
    PriorityQueue *pq = (struct PriorityQueue *) calloc(1, sizeof(PriorityQueue));
    pq->capacity = capacity;
    pq->size = 0; //Starts empty
    pq->node = (Node **) calloc(capacity, sizeof(Node *));

    return pq;
}

void pq_delete(PriorityQueue **q) {
    free((*q)->node);
    (*q)->node = NULL;
    free(*q);
}

bool pq_empty(PriorityQueue *q) {
    return (pq_size(q) == 0) ? true : false;
}

bool pq_full(PriorityQueue *q) {
    //Check if Last element of Queue is not empty <= I do not know how to do this
    //Less Efficient Alternative: If the queue is equal to capacity it is full
    if (pq_size(q) >= q->capacity)
        return true;
    return false;
}

uint32_t pq_size(PriorityQueue *q) {
    return q->size;
}

bool enqueue(PriorityQueue *q, Node *n) {
    if (pq_full(q))
        return false;

    q->node[pq_size(q)] = n; //Add Node to immediate right of the previous node
    q->size += 1; //Node is added size of queue increases

    //Insertion sort
    int size = pq_size(q);
    int j = 0;
    Node *tmp_node = NULL;
    for (int i = 1; i < size; i++) {
        tmp_node = q->node[i];
        j = i - 1;

        //Move elements of arr[0..i-1], that are greater than key, to one position ahead of their current position
        while (j >= 0 && q->node[j]->frequency < tmp_node->frequency) {
            q->node[j + 1] = q->node[j];
            j = j - 1;
        }
        q->node[j + 1] = tmp_node;
    }

    return true;
}

bool dequeue(PriorityQueue *q, Node **n) {
    if (pq_empty(q))
        return false;

    int size = pq_size(q);

    *n = q->node[size - 1];

    q->size -= 1; //Node is removed size of queue decreases
    return true;
}

void pq_print(PriorityQueue *q) {
    uint32_t nodes = pq_size(q);
    for (uint32_t n = 0; n < nodes; n++) {
        printf("Node %d\n", n);
    }
}
