#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "node.h"

Node *node_create(char *word) {
    Node *n = malloc(sizeof(Node)); //Memory for 1 node
    n->word = strdup(word);
    n->count = 0;
    //n->next = NULL; //Node pointer points to nothing //how tf do you make a linked list without changing node.h
    return n;
}

void node_delete(Node **n) {
    (*n)->word = NULL;
    free(*n);
    (*n) = NULL;
    //n = NULL;
}

void node_print(Node *n) {
    if (n == NULL) {
        printf("\n");
    } else {
        printf("'%s' f(%" PRIu32 ")\n", n->word, n->count);
    }
}
