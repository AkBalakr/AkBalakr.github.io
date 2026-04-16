#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <gmp.h>
#include <string.h>
#include "rsa.h"
#include "randstate.h"
#include "numtheory.h"

uint8_t set_bit(uint8_t byte, uint8_t index) {
    return byte |= (1 << index);
}

uint8_t clear_bit(uint8_t byte, uint8_t index) {
    return byte &= ~(1 << index);
}

uint8_t get_bit(uint8_t byte, uint8_t index) {
    return (byte >> index) & 1;
}

//Finds the number of bytes
uint64_t get_bit_count(mpz_t n) {
    uint64_t blocks = 1;
    mpz_t tmp_n;
    mpz_init(tmp_n);
    mpz_set(tmp_n, n);
    mpz_div_ui(tmp_n, tmp_n, 2);
    while (mpz_cmp_ui(tmp_n, 0) > 0) {
        blocks++;
        mpz_div_ui(tmp_n, tmp_n, 2);
    }
    mpz_clear(tmp_n);
    return blocks;
}

void rsa_make_pub(mpz_t p, mpz_t q, mpz_t n, mpz_t e, uint64_t nbits, uint64_t iters) {

    mpz_t phi;
    mpz_init(phi);
    mpz_t tmp_p;
    mpz_init(tmp_p);
    mpz_t tmp_q;
    mpz_init(tmp_q);
    mpz_t d;
    mpz_init(d);

    srand(time(NULL));
    int r = rand();
    uint64_t p_bits = r % (nbits / 2) + (nbits / 4); // Determining p_bits

    uint64_t q_bits = nbits - p_bits + 1; // remaining bits + 1 to q
    make_prime(p, p_bits, iters);
    make_prime(q, q_bits, iters);

    /* Calculate n = p x q */
    mpz_mul(n, p, q);

    /* Compute phi(n) = (p-1)(q-1) */
    mpz_sub_ui(tmp_p, p, 1);
    mpz_sub_ui(tmp_q, q, 1);
    mpz_mul(phi, tmp_p, tmp_q);

    /* Calculate d (multiplicative inverse of e mod phi) */
    mod_inverse(d, e, phi);
    if (mpz_cmp_ui(d, 0) == 0) {
        gcd(tmp_p, e, phi);
    }

    //Clear the mpz Variables
    mpz_clear(tmp_p);
    mpz_clear(tmp_q);
    mpz_clear(d);
    mpz_init(phi);
}

void rsa_write_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    //Format n, e, s, username, each with trailing newline n,e,s should be in hexstrings
    char ch = '\n';
    mpz_out_str(pbfile, 16, n);
    putc(ch, pbfile);
    mpz_out_str(pbfile, 16, e);
    putc(ch, pbfile);
    mpz_out_str(pbfile, 16, s);
    putc(ch, pbfile);
    fprintf(pbfile, "%s\n", username);
    putc(ch, pbfile);
}

void rsa_read_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {

    gmp_fscanf(pbfile, "%ZX", n);
    gmp_fscanf(pbfile, "%ZX", e);
    gmp_fscanf(pbfile, "%ZX", s);
    gmp_fscanf(pbfile, "%s\n", username);

    return;
}

void rsa_make_priv(mpz_t d, mpz_t e, mpz_t p, mpz_t q) {
    mpz_t tmp_p;
    mpz_init(tmp_p);
    mpz_sub_ui(tmp_p, p, 1); //tmp_p = p - 1
    mpz_t tmp_q;
    mpz_init(tmp_q);
    mpz_sub_ui(tmp_q, q, 1); //tmp_q = q - 1

    mpz_t lcm_pq;
    mpz_init(lcm_pq);

    lcm(lcm_pq, tmp_p, tmp_q);

    mod_inverse(d, e, lcm_pq);

    mpz_clear(lcm_pq);
    mpz_clear(tmp_p);
    mpz_clear(tmp_q);

    // Got p, e, q from make_pub
    // Find d such that, de ≅ 1 mod (λ(n)) //call modular inverse
    // From that find d
}

void rsa_write_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    char ch = '\n';
    mpz_out_str(pvfile, 16, n);
    putc(ch, pvfile);
    mpz_out_str(pvfile, 16, d);
    putc(ch, pvfile);
}

void rsa_read_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%ZX", n);
    gmp_fscanf(pvfile, "%ZX", d);
}

void rsa_encrypt(mpz_t c, mpz_t m, mpz_t e, mpz_t n) {
    // Encyrtion: c = m^e (mod n)
    pow_mod(c, m, e, n);
}

void rsa_encrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t e) {
    uint64_t blocks = 1;

    mpz_t tmp_n;
    mpz_init(tmp_n);
    mpz_set(tmp_n, n);
    mpz_div_ui(tmp_n, tmp_n, 2);
    while (mpz_cmp_ui(tmp_n, 0) > 0) {
        blocks++;
        mpz_div_ui(tmp_n, tmp_n, 2);
    }
    blocks = blocks / 8; //8 bits in a byte

    uint8_t *buffer = calloc(blocks + 1, sizeof(uint8_t));
    //uint8_t buffer[blocks];
    uint8_t buffin[blocks];
    ;
    uint64_t bytes_read;

    mpz_t chunk;
    mpz_init(chunk);
    char ch = '\n';
    bool start = false;
    do { // Clears the first 8 bits, logically useless but it fixed my problem
        for (uint8_t b = 0; b < blocks; b += 1) {
            for (int i = 0; i <= 7; i++) {
                buffin[b] = clear_bit(buffin[b], i);
                buffer[b] = clear_bit(buffer[b], i);
            }
        }

        bytes_read = fread(buffin, sizeof(uint8_t), blocks - 1, infile);
        if (bytes_read > 0) {
            if (start == true)
                putc(ch, outfile);
            if (start == false)
                start = true;
            // Add the 0xFF to the front to deal with the case of leading 0's
            buffer[0] = 0xFF;
            for (uint8_t b = 0; b < blocks; b += 1) {
                buffer[b + 1] = buffin[b];
            }

            mpz_import(chunk, blocks, 1, sizeof(uint8_t), 0, 0, buffer);
            rsa_encrypt(chunk, chunk, e, n);
            mpz_out_str(outfile, 16, chunk);
        }
    } while (bytes_read > 0);
    //Data in infile must be encrypted in blocks, because %n
    //Value of a block cannot be 0 or 1, solve this by prepending 0xFF */
}

void rsa_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t n) {
    //Decryption: m = c^d mod n
    pow_mod(m, c, d, n);
}

void rsa_decrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t d) {

    /* Block Size k = floor((LogBase2 (n) - 1)/8) 
    Allocate k bytes type (uint8_t *)
    Make hextring into a number
    Decrypt
    mpz_export(), convert c back into bytes
    fprintf(k - 1 bytes, outfile) */

    uint64_t blocks = 1;

    mpz_t tmp_n;
    mpz_init(tmp_n);
    mpz_set(tmp_n, n);
    mpz_div_ui(tmp_n, tmp_n, 2);
    while (mpz_cmp_ui(tmp_n, 0) > 0) {
        blocks++;
        mpz_div_ui(tmp_n, tmp_n, 2);
    }
    blocks = blocks / 8; //8 bits in a byte

    uint8_t *buffer = calloc(blocks, sizeof(uint8_t));
    uint8_t buffin[blocks];
    uint32_t chunks_read;

    uint64_t chunk_bytes;
    mpz_t chunk;
    mpz_init(chunk);
    size_t count;
    do {
        chunk_bytes = 0;

        chunks_read = gmp_fscanf(infile, " %ZX ", chunk);

        if (chunks_read == 1) {
            rsa_decrypt(chunk, chunk, d, n);

            for (uint8_t b = 0; b < blocks; b += 1) {
                for (int i = 0; i <= 7; i++) {
                    buffer[b] = clear_bit(buffer[b], i);
                }
            }

            mpz_export(buffer, &count, 1, sizeof(uint8_t), 1, 0, chunk);

            buffin[0] = clear_bit(buffer[0], 7);
            for (uint8_t b = 1; b < blocks; b += 1) {
                buffin[b - 1] = buffer[b];
            }
            fwrite(buffin, sizeof(uint8_t), count - 1, outfile);
        }

    } while (chunks_read == 1);
    mpz_clear(chunk);
    //Data in infile must be encrypted in blocks, because %n
    //Value of a block cannot be 0 or 1, solve this by prepending 0xFF
}

void rsa_sign(mpz_t s, mpz_t m, mpz_t d, mpz_t n) {
    /*
    signature s = (m^d) mod n
    d private key d //only you should have the private key
    Sign your username: akbalakr => m
    */
    pow_mod(s, m, d, n);
    return;
}

bool rsa_verify(mpz_t m, mpz_t s, mpz_t e, mpz_t n) {
    /* 1) s^e mod n  2) m = m’? */
    mpz_t se;
    mpz_init(se);
    mpz_t m1;
    mpz_init(m1);

    pow_mod(m1, s, e, n);
    if (mpz_cmp(m, m1) == 0) {
        return true;
    }

    mpz_clear(se);
    mpz_clear(m1);
    return false;
}
