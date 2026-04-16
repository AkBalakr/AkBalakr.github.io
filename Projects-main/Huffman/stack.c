#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "stack.h"
#include "node.h"

struct Stack {
    uint32_t top;
    uint32_t capacity;
    Node **items;
};

Stack *stack_create(uint32_t capacity) {

    struct Stack *stack = (struct Stack *) malloc(sizeof(struct Stack));
    stack->items = (Node **) malloc(sizeof(Node *) * capacity);
    stack->top = 0;
    stack->capacity = capacity;
    return stack;
}

void stack_delete(Stack **s) {
    free((*s)->items);
    (*s)->items = NULL;
    //(*s)->capacity = NULL;
    //(*s)->top = NULL;
    free(*s);
}

bool stack_empty(Stack *s) {
    return (s->top == 0);
}

bool stack_full(Stack *s) {
    return s->top == s->capacity;
}

uint32_t stack_size(Stack *s) {
    return s->top;
}

bool stack_push(Stack *s, Node *n) {
    if (stack_full(s)) {
        printf("Full Stack\n");
        return false; //Push was NOT successful, due to a full stack
    } else {
        s->top += 1; //Increment size
        s->items[s->top] = n; //Put value into empty space
        return true; //Push was successful
    }
}

bool stack_pop(Stack *s, Node **n) {

    if (stack_empty(s)) {
        printf("Empty Stack\n\n");
        return false;
    } else {
        *n = s->items[s->top]; //Take value form last element in stack
        s->top--; //Decrement size
        return true;
    }
}

void stack_print(Stack *s) {
    uint32_t i;
    if (stack_empty(s)) {
        printf("Empty Stack\n");
    } else {
        printf("The Stack Elements are : ");
        for (i = s->top; i > 0; i--)
            printf("%" PRIu64, s->items[i]->frequency);
        printf("%c->f(%" PRIu64 ") ", s->items[i]->symbol, s->items[i]->frequency);
        printf("\n");
    }
}
