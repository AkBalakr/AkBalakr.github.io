#include "bf.h"
#include "salts.h"
#include "speck.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

struct BloomFilter {
    uint64_t primary[2]; //primary hash fnction salt
    uint64_t secondry[2]; //secondry hash function salt
    uint64_t tertiary[2]; //tertiary hash function salt
    BitVector *filter;
};
uint32_t bf_size_bits = 0;
BloomFilter *bf_create(uint32_t size) {
    BloomFilter *bfp = malloc(sizeof(struct BloomFilter));
    bfp->primary[0] = SALT_PRIMARY_LO;
    bfp->primary[1] = SALT_PRIMARY_HI;

    bfp->secondry[0] = SALT_SECONDARY_LO;
    bfp->secondry[1] = SALT_SECONDARY_HI;

    bfp->tertiary[0] = SALT_TERTIARY_LO;
    bfp->tertiary[1] = SALT_TERTIARY_HI;

    bfp->filter = bv_create(size);
    bf_size_bits = size;
    return bfp;
}

void bf_delete(BloomFilter **bf) {
    bv_delete(&((*bf)->filter));
    free(*bf);
}

uint32_t bf_size(BloomFilter *bf) {
    uint32_t num = bv_length(bf->filter);
    //if(num > 0)  return num* 8; else return 8;
    return num;
}

void bf_insert(BloomFilter *bf, char *word) {
    uint32_t p = hash(bf->primary, word) % bf_size_bits;
    uint32_t s = hash(bf->secondry, word) % bf_size_bits;
    uint32_t t = hash(bf->tertiary, word) % bf_size_bits;
    //printf("bf_insert word = %s  p =  %" PRIu32 " s =  %"PRIu32"  t=  %"PRIu32" bf_size_bits =  %"PRIu32" \n", word, p, s,t, bf_size_bits);

    bv_set_bit(bf->filter, p);
    bv_set_bit(bf->filter, s);
    bv_set_bit(bf->filter, t);
    /*
  bool a = bv_get_bit(bf->filter, p);
  bool b = bv_get_bit(bf->filter, s);
  bool c = bv_get_bit(bf->filter, t);
  */
    //printf(" after setting   a = %i  b= %i  c = %i \n ", a, b, c);

    // printf("bf_insert created bv_set_bit");
}

bool bf_probe(BloomFilter *bf, char *word) {
    uint32_t p = hash(bf->primary, word) % bf_size_bits;
    uint32_t s = hash(bf->secondry, word) % bf_size_bits;
    uint32_t t = hash(bf->tertiary, word) % bf_size_bits;
    //printf(" bf_probe word = %s p =  %" PRIu32 " s =  %"PRIu32"  t=  %"PRIu32" bf_size_bits =  %"PRIu32" \n",word, p, s,t, bf_size_bits);
    bool a = bv_get_bit(bf->filter, p);
    bool b = bv_get_bit(bf->filter, s);
    bool c = bv_get_bit(bf->filter, t);

    //printf("bloom   a = %i  b= %i  c = %i \n ", a, b, c);
    if ((a & b & c) == true)
        return true;
    else
        return false;
}

void bf_print(BloomFilter *bf) {
    bv_print(bf->filter);
}
