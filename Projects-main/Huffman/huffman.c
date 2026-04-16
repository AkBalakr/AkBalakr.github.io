#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "huffman.h"
#include "header.h"
#include "pq.h"
#include "node.h"
#include "code.h"
#include "io.h"
#include "stack.h"

static Code c;

Node *build_tree(uint64_t hist[static ALPHABET]) {
    /*
    (a) Create a priority queue.
    For each symbol histogram where its frequency is greater than 0 (there should be at minimum two elements because of 
    step 2), create a corresponding Node and insert it into the priority queue.
    (b) While there are two or more nodes in the priority queue, dequeue two nodes.
    The first de- queued node will be the left child node. The second dequeued node will be the right child node.
    Join these nodes together using node_join() and enqueue the joined parent node. 
    The frequency of the parent node is the sum of its left child’s frequency and its right child’s frequency.
    (c)  Eventually, there will only be one node left in the priority queue. This node is the root of the constructed Huffman tree. 
    */

    Node *root = NULL;
    Node *node = NULL;
    Node *left = NULL;
    Node *right = NULL;
    Node *parent = NULL;
    bool flag = false;

    PriorityQueue *pq = pq_create(ALPHABET);
    for (int i = 0; i < ALPHABET; i++) { //Loop through aphabets
        if (hist[i] > 0) { //If the frequency of a character is more than 0
            node = node_create(i, hist[i]); //Create a node
            enqueue(pq, node); //Add the node to the queue
        }
    }
    while (pq_size(pq) >= 2) { //pq not only the root
        flag = dequeue(pq, &left); //Add to left first remove from queue
        flag = dequeue(pq, &right); //Add to right second remove from queue

        parent = node_join(left, right); //Make a parent node for left and right
        enqueue(pq, parent); //Add the parent
    }

    pq_empty(pq);
    dequeue(pq, &root);
    c = code_init();
    pq_delete(&pq);
    return root;
}

void build_codes(Node *root, Code table[static ALPHABET]) {

    if (root != NULL) { //If there is a Root
        //printf("Root is not NULL\n");
        if ((root->left == NULL) && (root->right == NULL)) { //Leaf = No Children. Done
            //printf("\n");
            //node_print(root);
            //printf("\nLeaf(%c) ", root->symbol);
            //code_print(&c);
            table[root->symbol] = c;
            //code_print(&table[root->symbol]);
        } else {
            //printf("Not leaf\n");
            uint8_t p_bit; //Stores the popped bit
            if (!(root->right == NULL)) {
                code_push_bit(&c, 0);
                build_codes(root->left, table); //If they are a LEFT child, add 0 to code
                code_pop_bit(&c, &p_bit);
            }
            //printf("%d", p_bit);
            if (!(root->right == NULL)) {
                code_push_bit(&c, 1);
                build_codes(root->right, table); //If they are a RIGHT child, add 1 to code
                code_pop_bit(&c, &p_bit);
            }
        }
    }
    //else {  printf("Root is null\n"); }
    //printf("End of build_codes\n");
}

void dump_tree(int outfile, Node *root) {
    uint8_t L = 'L';
    uint8_t I = 'I';
    if (root != NULL) {
        dump_tree(outfile, root->left);
        dump_tree(outfile, root->right);
        if ((root->left == NULL) && (root->right == NULL)) {
            //LEAF node
            //need to implement  write
            //printf("L");
            //printf("%c", root->symbol);
            write_bytes(outfile, &L, 1);
            uint8_t buffer[2] = "\0";
            buffer[0] = root->symbol;
            write_bytes(outfile, buffer, 1);
        } else {
            //Interior node
            //printf("I");
            write_bytes(outfile, &I, 1);
        }
    }
}

Node *rebuild_tree(uint16_t nbytes, uint8_t tree[static nbytes]) {

    /*
    (a) The array containing the dumped tree will be referred to as tree_dump.
        The length of this array will be nbytes. A stack of nodes will be needed to reconstruct the tree.
    (b) Iterate over the contents tree_dump from 0 to nbytes.
    (c) If the element of the array is an ‘L’, 
        then the next element will be the symbol for the leaf node. Use that symbol to create a new node with node_create(). 
        Push the created node onto the stack.
    (d) If the element of the array is an ‘I’, then you have encountered an interior node. 
        Pop the stack once to get the right child of the interior node, then pop again to get the left child of the interior node.
        Note: the pop order is important. Join the left and right nodes with node_join() and push the joined parent node on the stack.
    (e) There will be one node left in the stack after you finish iterating over the contents tree_dump. 
        This node is the root of the Huffman tree.
    */
    Stack *stack = stack_create(nbytes);
    Node *root = NULL;
    Node *left = NULL;
    Node *right = NULL;
    Node *parent = NULL;
    uint16_t i = 0;

    while (i < nbytes) {
        if (tree[i] == 'L') { //Leaf Node
            i++;
            left = node_create(tree[i], 0); //Create a Leaf Node
            stack_push(stack, left);
        } else if (tree[i] == 'I') { //Intermediate Node
            stack_pop(stack, &right);
            stack_pop(stack, &left);
            parent = node_join(left, right);
            stack_push(stack, parent);
        }
        i += 1;
    }
    stack_pop(stack, &root);
    stack_delete(&stack);
    return root;
}

void delete_tree(Node **root) {
    //Post Order Deleting of the Tree
    if (*root != NULL) {
        delete_tree(&((*root)->left));
        delete_tree(&((*root)->right));
        node_delete(root);
    }
}
