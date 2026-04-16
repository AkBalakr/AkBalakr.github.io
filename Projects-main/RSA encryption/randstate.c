#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <gmp.h>

#include "randstate.h"

gmp_randstate_t state;

void randstate_init(uint64_t seed) {
    srandom(seed); // Initialization, should only be called once
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, seed);
    return;
}

void randstate_clear(void) {
    gmp_randclear(state);
    return;
}
