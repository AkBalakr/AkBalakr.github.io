#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "code.h"

Code code_init(void) {
    /*
This function will not require any dynamic memory allocation. 
You will simply create a new Code on the stack, setting top to 0, 
and zeroing out the array of bits, bits. The initialized Code is then returned.
*/

    Code c;

    //Setting all the bits to 0
    for (uint32_t i = 0; i < MAX_CODE_SIZE; i++) {
        c.bits[i] = 0;
    }

    c.top = 0; // top is bits[0] if nothing is there
    return c;
}

uint32_t code_size(Code *c) {
    //Returns the size of the Code, which is exactly the number of bits pushed onto the Code.
    uint32_t size = c->top;

    return size;
}

bool code_empty(Code *c) {
    //Returns true if the Code is empty and false otherwise.
    if (code_size(c) == 0)
        return true;
    return false;
}

bool code_full(Code *c) {
    /*
	Returns true if the Code is full and false otherwise. 
	The maximum length of a code in bits is 256, which we 
	have defined using the macro ALPHABET. Why 256? Because 
	there are exactly 256 ASCII characters (including the extended ASCII).
	*/
    if (code_size(c) == MAX_CODE_SIZE)
        return true;
    return false;
}

bool code_set_bit(Code *c, uint32_t i) {
    /*
	Sets the bit at index i in the Code, setting it to 1. If i is out of range,
	return false. Otherwise, return true to indicate success.
	*/
    if (i >= MAX_CODE_SIZE) {
        printf("Out of range: code_SET_bit\n");
        return false; //Out of Range
    }
    //Setting the it to 1?
    c->bits[i] = 1;
    return true;
}

bool code_clr_bit(Code *c, uint32_t i) {
    /*
	Clears the bit at index i in the Code, clearing it to 0. If i is out of range, 
	return false. Otherwise, return true to indicate success.
	*/
    if (i >= MAX_CODE_SIZE) {
        printf("Out of range: code_CLR_bit\n");
        return false; //Out of Range
    }

    c->bits[i] = 0; //Setting the it to 0?
    return true;
}

bool code_get_bit(Code *c, uint32_t i) {
    /*
	Gets the bit at index i in the Code. If i is out of range, or if bit i is equal to 0, 
	return false. Return true if and only if bit i is equal to 1.
	*/

    if (i >= MAX_CODE_SIZE) {
        printf("Out of range: code_GET_bit\n");
        return false; //Out of Range
    }

    return c->bits[i] == 1; //Returns true: 1, false: anything else
}

bool code_push_bit(Code *c, uint8_t bit) {
    /*
	Pushes a bit onto the Code. The value of the bit to push is given by bit. 
	Returns false if the Code is full prior to pushing a bit and true otherwise to
	indicate the successful pushing of a bit.
	*/
    if (code_size(c) >= MAX_CODE_SIZE) {
        printf("Out of range: code_PUSH_bit\n");
        return false; //Out of Range
    }

    c->bits[code_size(c)] = bit;
    c->top += 1;
    return true;
}

bool code_pop_bit(Code *c, uint8_t *bit) {
    /*
	Pops a bit off the Code. The value of the popped bit is passed back 
	with the pointer bit. Returns false if the Code is empty prior to popping a bit 
	and true otherwise to indicate the successful popping of a bit.
	*/

    if (code_size(c) >= MAX_CODE_SIZE) {
        printf("Out of range: code_PUSH_bit\n");
        return false; //Out of Range
    }
    uint32_t size = code_size(c);
    *bit = c->bits[size - 1]; // Putting the value into bit
    code_clr_bit(c, size - 1);
    c->top -= 1; // Moving top down after pop
    return true;
}

void code_print(Code *c) {
    uint32_t size = code_size(c);
    printf("code(bits:%d)[ ", size);
    for (uint32_t i = 0; i < size; i++) {
        printf("%d", c->bits[i]);
    }
    printf(" ]\n");
}
