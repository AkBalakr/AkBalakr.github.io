#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "bv.h"

struct BitVector {
    uint32_t length;
    uint8_t *vector;
};

uint8_t set_bit(uint8_t byte, uint8_t index) {
    return byte |= (1 << index);
}

uint8_t clear_bit(uint8_t byte, uint8_t index) {
    return byte &= ~(1 << index);
}

uint8_t get_bit(uint8_t byte, uint8_t index) {
    return (byte >> index) & 1;
}

BitVector *bv_create(uint32_t length) {
    BitVector *bv = malloc(sizeof(struct BitVector));
    bv->length = length;
    bv->vector = calloc(bv_length(bv), sizeof(uint8_t));
    return bv;
}

void bv_delete(BitVector **bv) {
    free((*bv)->vector);
    free(*bv);
}

uint32_t bv_length(BitVector *bv) {
    return (bv->length / 8) + 1;
}

bool bv_set_bit(BitVector *bv, uint32_t i) {
    //printf("<------bv->length %"PRIu32"  i = %"PRIu32" ------>\n", bv->length, i);
    if (i >= bv->length) {
        printf("Out of range: bv_set_bit\n");
        return false; //Out of Range
    }

    bv->vector[i / 8] = set_bit(bv->vector[i / 8], i % 8); //[0000 0001]
    return true;
}

bool bv_clr_bit(BitVector *bv, uint32_t i) {
    /*
	Clears the bit at index i in the Code, clearing it to 0. If i is out of range, 
	return false. Otherwise, return true to indicate success.
	*/
    if (i >= bv_length(bv)) {
        printf("Out of range: bv_clr_bit\n");
        return false; //Out of Range
    }
    bv->vector[i / 8] = clear_bit(bv->vector[i / 8], i % 8) == 0; //[0000 0001]
    return true;
}

bool bv_get_bit(BitVector *bv, uint32_t i) {
    if (i >= bv->length) {
        printf("Out of range: bv_get_bit\n");
        return false; //Out of Range
    }
    return get_bit(bv->vector[i / 8], i % 8) == 1; //[0000 0001]
}

void bv_print(BitVector *bv) {
    uint8_t num = *(bv->vector);
    printf(":");
    printf("Length = %" PRIu32 " Vector  %" PRIu8 " \n", bv->length, num);
}
