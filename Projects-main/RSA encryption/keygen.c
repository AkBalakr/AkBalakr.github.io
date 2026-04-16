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
    fprintf(stderr, "usage: keygen [-b:i:s:vhn:d:]\n");
    fprintf(stderr,
        "-b:specifies the minimum bits needed for the public modulus n.\t\n-i:specifies the number "
        "of Miller-Rabin iterations for testing primes(default:50).\t\n-n pbfile :specifies the  "
        "public key file(default:rsa.pub). \t\n-d pvfile: specifies the private "
        "keyfile(default:rsa.priv).\t\n-s : specifies the random seed for the random state "
        "initialization (default: the seconds since the UNIX epoch, given by time(NULL)).\t\n "
        "-v:enables verbose output.-h:displays program synopsis and usage.\t\n");
    exit(EXIT_FAILURE);
}

char *get_current_user() {
    char *rtnPtr = getenv("USER");
    return rtnPtr;
}

int main(int argc, char **argv) {
    FILE *public_fp;
    FILE *private_fp;
    char *public_key_filename = "rsa.pub";
    char *private_key_filename = "rsa.priv";
    bool minimum_bits = false; //default public modulus n
    bool interaction_prime = true; //default is to have ncurses
    uint64_t seed = 1;
    bool seed_flag = false;
    int iterations = 50;
    bool verbose = false;
    uint64_t n_bits = 128; //User input
    size_t e_value = 65537; /* public exponent */
    int ch;
    while ((ch = getopt(argc, argv, "b:i:s:vhn:d:")) != -1) {
        switch (ch) {
        case 'b':
            minimum_bits = true; //specifies the minimum bits needed for the public modulus n
            n_bits = (uint64_t) strtol(optarg, NULL, 10);
            //n_bits = atoi(optarg);
            break;
        case 'i': //specifies the number of Miller-Rabin iterations for testing primes(default:50)
            interaction_prime = true;
            //iterations = (uint32_t) strtol(optarg, NULL, 10);
            iterations = atoi(optarg);
            break;
        case 'n':
            //generations = (uint32_t) strtol(optarg, NULL, 10);
            //specifies the public keyfile
            public_key_filename = optarg;
            break;
        case 'd':
            private_key_filename = optarg; //specifies the private keyfile
            break;
        case 's': //specifies the random seed for the random state initialization (default: the seconds since the UNIX epoch, given by time(NULL)).
            seed_flag = true;
            seed = atoi(optarg); //put the last generation not in stdout
            //srand(time(NULL));   // Initialization, should only be called once.
            //rand_value = rand();      // Returns a pseudo-random integer between 0 and RAND_MAX.
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

    /* 2. Open the public and private key files using fopen(). 
        Print a helpful error and exit the program in
        the event of failure. */

    if (!(public_fp = fopen(public_key_filename, "w"))) {
        perror(public_key_filename);
        return 1;
    }

    if (!(private_fp = fopen(private_key_filename, "w"))) {
        perror(private_key_filename);
        return 1;
    }

    //printf("Fileno public_fp %d \n", fileno(public_fp));
    //printf("Fileno private_fp %d \n", fileno(private_fp));

    /* 
    3. Using fchmod() and fileno(), make sure that the private key file permissions are set to 0600,
     indicating read and write permissions for the user, and no permissions for anyone else.

    File permission changed to -rw-------  ( 600 ) read and write access to the user 
    While the file is originally created -rw-r--r--  */

    int fildes;
    fildes = open(public_key_filename, O_RDWR);
    fchmod(fildes, S_IRUSR | S_IWUSR);
    close(fildes);

    fildes = open(private_key_filename, O_RDWR);
    fchmod(fildes, S_IRUSR | S_IWUSR);
    close(fildes);

    //printf("public_fp %d \n", fileno(public_fp));
    //printf("private_fp %d \n", fileno(private_fp));

    /* 4. Initialize the random state using rand state_init(),
    using the set seed. */
    /* if (seed_flag == false) {
        srand(time(NULL)); 
        r = rand();
    } 
    else r = rand_value;*/

    randstate_init(seed);

    mpz_t p;
    mpz_init(p);
    mpz_t q;
    mpz_init(q);
    mpz_t n;
    mpz_init(n);
    mpz_t e;
    mpz_init(e);
    mpz_set_ui(e, e_value); //e = e_value
    mpz_t d;
    mpz_init(d);
    mpz_t m;
    mpz_init(m);

    uint64_t iters = iterations; //User input iterations

    /*5. Make the public and private keys using rsa_make_pub() and
  rsa_make_priv(),respectively.*/

    rsa_make_pub(p, q, n, e, n_bits, iters);
    rsa_make_priv(d, e, p, q);

    //6. Get the current user’s name as a string.You will want to use getenv().
    char *username = calloc(128, sizeof(char));
    username = get_current_user();

    /*7. Convert the username into an mpz_t with mpz_set_str(), 
    specifying the base as 62.
    Then, use rsa_sign() to compute the signature of the username. */

    mpz_t signature;
    mpz_init(signature);

    mpz_set_str(signature, username, 62); // makes signature a string of digits base base

    rsa_sign(signature, signature, d, n);

    // 8. Write the computed public and private key to their respective files.
    rsa_write_pub(n, e, signature, username, public_fp);
    rsa_write_priv(n, d, private_fp);

    /* 9. If verbose output is enabled print the following,
    each with a trailing newline, inorder: */
    if (verbose) {
        printf("(a) username %s\n", username);
        gmp_printf(
            "(b) the signature s  (%" PRIu64 ")  %ZX\n", get_bit_count(signature), signature);
        gmp_printf("(c) the first large prime p (%" PRIu64 ") %ZX\n", get_bit_count(p), p);
        gmp_printf("(d) the second large prime q  (%" PRIu64 ") %ZX\n", get_bit_count(q), q);
        gmp_printf("(e) the public modulus n  (% " PRIu64 ") %ZX\n", get_bit_count(n), n);
        gmp_printf("(f) the public exponent e  (%" PRIu64 ") %ZX\n", get_bit_count(e), e);
        gmp_printf("(g) the privatekey d (%" PRIu64 ") %ZX\n", get_bit_count(d), d);
    }

    /*
    Close the public and privatekey files,clear the random state with 
    randstate_clear(),and clear any mpz_t variables you may have used. */
    fclose(public_fp);
    fclose(private_fp);
    randstate_clear();

    /* free */
    mpz_clear(signature);
    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(n);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(m);

    return 0;
}
