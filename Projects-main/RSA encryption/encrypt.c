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

void usage(void) {
    fprintf(stderr, "usage: encrypt [-ionvh]\n");
    fprintf(stderr, "-i:specifies the inputfile to encrypt(default:stdin).\t\n-o:specifies the "
                    "outputfile to encrypt(default:stdout).\t\n-n specifies the file containing "
                    "the publickey(default:rsa.pub). \t\n-v:enables verbose output.\t\n-h:displays "
                    "program synopsis and usage.\t\n");
    exit(EXIT_FAILURE);
}
int main(int argc, char **argv) {
    FILE *public_fp;
    FILE *input_fp = stdin;
    FILE *output_fp = stdout;
    char *public_key_filename = "rsa.pub";
    char *input_filename = "files/encrypt_in.txt";
    char *output_filename = "files/encrypt_out.txt";
    bool input_file = false;
    bool output_file = false;
    bool verbose = false;
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
            //specifies the public keyfile
            public_key_filename = optarg;
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

    /* 2. Open the public key files using fopen(). 
        Print a helpful error and exit the program in
        the event of failure. */

    if (!(public_fp = fopen(public_key_filename, "r"))) {
        perror(public_key_filename);
        return 1;
    }

    /* 3. Read the public key from the opened public keyfile. */

    mpz_t n;
    mpz_init(n);
    mpz_t e;
    mpz_init(e);
    mpz_t s;
    mpz_init(s);

    char *username = calloc(128, sizeof(char));
    rsa_read_pub(n, e, s, username, public_fp);

    // If verbose output is enabled print the following, each with a trailing newline, inorder
    // number of bits that constitute them, along with their respective values in decimal
    if (verbose) {
        printf("(a) username %s\n", username);
        gmp_printf("(b) the signature s   (%" PRIu64 ") %ZX \n", get_bit_count(s), s);
        gmp_printf("(c) the public modulus n  (%" PRIu64 ") %ZX\n", get_bit_count(n), n);
        gmp_printf("(d) the public exponent e  (%" PRIu64 ") %ZX\n", get_bit_count(e), e);
    }

    /* Convert the username that was read into anmpz_t.
    This will be the expected value of the verified signature. Verify the signature using rsa_verify(), 
    reporting an error and exiting the program if the signature couldn’t be verified. */

    mpz_t signature1;
    mpz_init(signature1);
    mpz_set_str(signature1, username, 62); // makes signature a string of digits base base
    bool verified = rsa_verify(signature1, s, e, n);
    printf("%s\n", verified ? "Verified" : "Verified Failed");

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
            perror("Stdin no input");
            return (1);
        }
    }

    if (output_file) {
        if (!(output_fp = fopen(output_filename, "w"))) {
            perror(output_filename);
            return 1;
        }
    }

    rsa_encrypt_file(input_fp, output_fp, n, e);

    fclose(public_fp);
    fclose(input_fp);
    fclose(output_fp);

    // clear the mpz variables;
    mpz_clear(s);
    mpz_clear(signature1);
    mpz_clear(n);
    mpz_clear(e);
    free(username);
    return 0;
}
