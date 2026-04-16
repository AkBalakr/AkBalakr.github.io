#include "io.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "defines.h"
#include "io.h"
#include "code.h"
#include <inttypes.h>
#include <stdbool.h>
#define BUFFER_SIZE BLOCK

uint64_t bytes_read;
uint64_t bytes_written;

//bool DONE = false;
static uint32_t ind; // Tracks which bit we 've buffered up to
static uint8_t buffer[BUFFER_SIZE]; // Buffer .
//static bool zeroing = false;

uint8_t clear_bit(uint8_t byte, uint8_t index) {
    return byte &= ~(1 << index);
}

void print_buffer(char *str) {
    printf(" %s buffers [", str);
    for (uint32_t bl = 0; bl < BUFFER_SIZE; bl++) { // Loop through blocks
        for (uint32_t b = 8; b > 0; b--) {
            printf("%d", (buffer[bl] >> (b - 1) & 1));
        }
        printf(" ");
    }
    printf("]\n");
}

int read_bytes(int infile, uint8_t *buf, int nbytes) {

    ssize_t read_bytes_file;

    if ((read_bytes_file = read(infile, buf, nbytes)) < 0) {
        write(2, "An error occurred in the read.\n", 31);
        return -1;
    }

    bytes_read = nbytes;
    //printf(">>> nbytes = %d  read_bytes_file = %zd  %" PRIu64 " \n ", nbytes, read_bytes_file, bytes_read);

    return read_bytes_file;
}

int write_bytes(int outfile, uint8_t *buf, int nbytes) {
    if (outfile < 0) {
        return -1;
    }

    if (write(outfile, buf, nbytes) != nbytes) {
        write(2, "There was an error writing to testfile.txt\n", 43);
        return -1;
    }
    return 0;
}

bool read_bit(int infile, uint8_t *bit) {
    // Fill buffer if it's empty
    // Mark the first invalid bit

    if ((bytes_read == 0) || (bytes_written >= bytes_read) || (bytes_written >= sizeof(buffer))) {

        read_bytes(infile, buffer, BUFFER_SIZE);
        bytes_written = 0;
    }

    if (bytes_written < bytes_read) {
        *bit = buffer[(bytes_written)];
        bytes_written += 1;
        return false;
    }

    return true;
}

void write_code(int outfile, Code *c) {
    uint32_t size_c = code_size(c); //size of code
    uint8_t bit; //current bit of code
    uint32_t ind_c = 0;
    while (ind_c < size_c) {
        //getting the bit of the code
        if (ind <= BUFFER_SIZE * 8) { //if buffer is not full
            bit = c->bits[ind_c];
            buffer[ind / 8] = (buffer[ind / 8] << 1) | bit; //Shift the buffer
            ind++;
            ind_c++;
        }
        if (ind == BUFFER_SIZE * 8) {
            flush_codes(outfile); //When buffer is full flush
        }
    }
}

void flush_codes(int outfile) {
    //If I needed start adding zeros, done
    int bytes = (ind + 7) / 8;
    write_bytes(outfile, buffer, bytes);

    memset(buffer, 0, sizeof(buffer));
    ind = 0; //resetting index of bit
    /*
    if (ind == (BUFFER_SIZE * 8)) { // If the buffer is full flush normally
      // Convert ind to bytes.
      // ind == 7 -> 1 byte
      // ind == 8 -> 1 byte
      // ind == 9 -> 2 bytes

      int bytes = (ind + 7) / 8;
      write_bytes(outfile, buffer, bytes);
      memset(buffer, 0, sizeof(buffer));
      ind = 0; //resetting index of bit
    } else {
      Code zeros = code_init();
      int val = BUFFER_SIZE * 8;
      int final_val = (val) - (ind % val);
      for (uint32_t z = bytes_written; z < final_val; z++) { //Make a fake code of only 0's
          if (code_full(&zeros)) {
            bytes_written += z;
            write_code(outfile, &zeros);
            zeros = code_init();
          }
          code_push_bit(&zeros, 0);
      }
      zeroing = true;
      write_code(outfile, &zeros);
    }
    */
}
