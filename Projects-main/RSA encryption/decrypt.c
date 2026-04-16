#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <sys/stat.h>
#include <gmp.h>
#include "rsa.h"
#include "randstate.h"
#include "numtheory.h"

void usage(void) {
    fprintf(stderr, "usage: decrypt [-ionvh]\n");
    fprintf(stderr, "-i:specifies the inputfile to decrypt(default:stdin).\t\n-o:specifies the "
                    "outputfile to decrypt(default:stdout).\t\n-n specifies the file containing "
                    "the private key(default:rsa.priv). \t\n-v:enables verbose "
                    "output.\t\n-h:displays program synopsis and usage.\t\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    FILE *private_fp;
    FILE *input_fp = stdin;
    FILE *output_fp = stdout;
    char *private_key_filename = "rsa.priv";
    char *input_filename = "files/encrypt_out.txt";
    char *output_filename = "files/outputfinal.txt";
    bool verbose = false;
    bool input_file = false;
    bool output_file = false;
    int ch;
    while ((ch = getopt(argc, argv, "i:o:n:vh")) != -1) {
        switch (ch) {
        case 'i': //specifies the number of Miller-Rabin iterations for testing primes(default:50)
            input_filename = optarg; //specifies the input to the encrypted
            input_file = true;
            break;
        case 'o':
            output_filename = optarg; //specifies the private keyfile
            output_file = true;
            break;
        case 'n':
            //specifies the private keyfile
            private_key_filename = optarg;
            break;
        case 'v': //enables verbose output.
            verbose = true;
            break;
        case 'h': //displays program synopsis and usage.
            usage();
            break;
        default: break;
        }
    }

    /* 2. Open the private key files using fopen(). 
        Print a helpful error and exit the program in
        the event of failure. */

    if (!(private_fp = fopen(private_key_filename, "r"))) {
        perror(private_key_filename);
        return 1;
    }

    /* 3. Read the private key from the opened private keyfile. */

    mpz_t n;
    mpz_init(n);
    mpz_t d;
    mpz_init(d);
    rsa_read_priv(n, d, private_fp);

    // If verbose output is enabled print the following, each with a trailing newline, inorder
    // number of bits that constitute them, along with their respective values in decimal
    if (verbose) {
        gmp_printf("(a) the public modulus n  (%" PRIu64 ") %ZX\n", get_bit_count(n), n);
        gmp_printf("(b) the public exponent d  (%" PRIu64 ") %ZX\n", get_bit_count(d), d);
    }

    if (input_file) {
        if (!(input_fp = fopen(input_filename, "r"))) {
            perror(input_filename);
            return 1;
        }
    } else if (input_fp == NULL) {
        perror("Error opening file");
        return (1);
    } else {
        fseek(input_fp, 0, SEEK_END);
        int len = ftell(input_fp);
        printf("Total size of file.txt = %d bytes\n", len);
        if (len > 0) {
            fseek(input_fp, 0, SEEK_SET);
        } else {
            perror("Stdin no input - keygen");
            return (1);
        }
    }

    if (output_file) {
        if (!(output_fp = fopen(output_filename, "w"))) {
            perror(output_filename);
            return 1;
        }
    }

    rsa_decrypt_file(input_fp, output_fp, n, d);

    fclose(private_fp);
    fclose(input_fp);
    fclose(output_fp);

    // clear the mtz variables;
    mpz_clear(n);
    mpz_clear(d);
}
