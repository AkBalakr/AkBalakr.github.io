#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "node.h"

Node *node_create(uint8_t symbol, uint64_t frequency) {
    struct Node *newNode = malloc(sizeof(struct Node));
    newNode->symbol = symbol;
    newNode->frequency = frequency;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

void node_delete(Node **n) {
    if ((*n)->left == NULL && (*n)->right == NULL) {
        free(*n);
    }
}

Node *node_join(Node *left, Node *right) {
    uint64_t frequency = left->frequency + right->frequency;
    //printf("\nParent Join ->f(%" PRId64 ")\n", frequency);
    //printf("\tleft->f(%" PRId64 ") right->f(%" PRId64 ") ", left->frequency, right->frequency);

    uint8_t symbol = '$';
    Node *parent = node_create(symbol, frequency);
    parent->left = left;
    parent->right = right;
    return parent;
}

void node_print(Node *n) {
    printf("%c->f(%" PRId64 ") ", n->symbol, n->frequency);
}
