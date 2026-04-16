#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "ht.h"
#include "node.h"
#include "speck.h"

bool DELETE = false;
struct HashTable {
    uint64_t salt[2];
    uint32_t size;
    uint32_t n_items;
    Node **slots;
};

struct HashTableIterator {
    HashTable *table;
    uint32_t slot;
};

HashTable *ht_create(uint32_t size) {
    HashTable *ht = (HashTable *) malloc(sizeof(HashTable));
    ht->size = size;
    ht->n_items = 0;
    ht->slots = (Node **) calloc(sizeof(Node *), size);
    return ht;
}

void ht_delete(HashTable **ht) {
    //printf("Start ht Delete\n");
    HashTableIterator *hti = hti_create(*ht);
    //printf("Start Deleting nodes\n");
    DELETE = true;
    //printf("ht iter %s\n", ht_iter(hti));
    Node *del_node = ht_iter(hti);
    //node_print(del_node);

    while (del_node != NULL) {
        //printf("Delete Node\n");
        //node_print(del_node);
        node_delete(&del_node);
        del_node = ht_iter(hti);
    }
    //	printf("Deleted all Nodes in Slots\n");

    hti_delete(&hti);
    free((*ht)->slots);
    (*ht)->slots = NULL;
    free(*ht);
}

uint32_t ht_size(HashTable *ht) {
    return ht->n_items;
}

uint32_t find_slot(HashTable *ht, char *word) {
    uint32_t i = hash(ht->salt, word) % (ht->size);
    // search until we either find the key, or find an empty slot.

    //printf(" find_slot look up Key %"PRIu32" Items in HashTable\n", i);

    while ((ht->slots[i] != NULL) && ((strcmp(ht->slots[i]->word, word) != 0))) {
        //printf("Find_slot Equal - link_node->word = %s ||| word = %s \n ",ht->slots[i]->word, word);
        i = (i + 1) % (ht->size - 1);
    }

    return i;
}

Node *ht_insert(HashTable *ht, char *word) {

    if (ht->slots == NULL) {
        printf("ht slots NULL\n");
        return NULL;
    }
    uint32_t key = find_slot(ht, word);
    //uint32_t key = hash(ht->salt, word) % (ht->size); //index of ht for word

    //printf("Node[3] (%s) ", word); node_print(ht->slots[3]);
    Node *link_node = NULL;
    link_node = ht->slots[key]; //Get a node if there is one, should be 0 otherwise from calloc
    //printf("k[%"PRIu32"] ", key);

    if (link_node == NULL) {
        //printf("N-node: ");
        Node *node = node_create(word);
        ht->slots[key] = node;
        ht->slots[key]->count = 1;
        //node_print(node);
        ht->n_items++;
        //ht_print(ht);
        return ht->slots[key]; //return inserted node
    }

    if (link_node != NULL) {
        if (strcmp(ht->slots[key]->word, word) == 0) {
            //printf("Equal - link_node->word = %s ||| word = %s \n ",link_node->word, word);
            link_node->count = link_node->count + 1;
            //node_print(link_node);
            return link_node;
        }
    }
    return link_node; //return inserted node
}

Node *ht_lookup(HashTable *ht, char *word) {

    uint32_t key = find_slot(ht, word); //index of ht for word

    //printf(" look up Key %"PRIu32" Items in HashTable\n", key);
    Node *look_node = NULL;
    look_node = ht->slots[key];
    if (look_node == NULL)
        return NULL;

    if (strcmp(look_node->word, word) == 0) {
        //printf(" look up Key %"PRIu32" Items in HashTable %s  count %"PRIu32" \n", key, look_node->word, look_node->count);
        return look_node;
    } else
        return NULL;

    return NULL; //Nothing found
}

void ht_print(HashTable *ht) {
    printf("Print HashTable\n");
    HashTableIterator *hti = hti_create(ht);
    //uint32_t key = 0;
    //int32_t items = 0; //Items iterated over
    printf("NODES\n");

    /*
	printf("%"PRIu32" Items in HashTable\n", ht->n_items);
	//printf("NODE 1 "); node_print(ht->slots[1]);
	for(uint32_t i = 0; i < ht->size; i++) {
	//for(uint32_t i = 0; i < 20; i++) {
		Node *n = ht->slots[i];
		printf("	k[%"PRIu32"] ", i);
		node_print(n);
	}*/

    // printf("\n-----------Start HASHTABLE \n");
    Node *n = NULL;
    while ((n = ht_iter(hti)) != NULL) {
        printf(" \n K[%" PRIu32 "]", hti->slot - 1);
        node_print(n);
    }
    //printf("\n--------------STOP HASHTABLE \n");
    node_delete(&n);
    hti_delete(&hti);
    //printf("DONE print HashTable\n");
}

HashTableIterator *hti_create(HashTable *ht) {
    HashTableIterator *hti = (HashTableIterator *) malloc(sizeof(HashTableIterator));
    hti->slot = 0;
    hti->table = ht;
    return hti;
}

void hti_delete(HashTableIterator **hti) {
    free(*hti);
    *hti = NULL;
}

Node *ht_iter(HashTableIterator *hti) {

    while (
        hti->table->slots[hti->slot] == NULL) { //Keep looping until the value at slot is not null
        if (hti->slot >= hti->table->size) {
            return NULL;
        }
        hti->slot++; //Move to the next index
    }

    hti->slot++; //Increment so it does not start on itself which is already a non-NULL node
    return hti->table->slots[hti->slot - 1];
}
