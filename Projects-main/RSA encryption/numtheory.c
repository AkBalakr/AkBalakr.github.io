#include <gmp.h>
#include "numtheory.h"
#include "randstate.h"

void lcm(mpz_t d, mpz_t a, mpz_t b) {
    mpz_t n1;
    mpz_init(n1);
    gcd(d, a, b);
    mpz_mdiv(d, a, d);
    mpz_mul(d, d, b);
    mpz_clear(n1);
    return;
}

void gcd(mpz_t d, mpz_t a, mpz_t b) {
    //mpz_gcd(d, a, b); //MAKE YOUR OWN LATER

    mpz_t t, one, a1, b1;
    mpz_init(t);
    mpz_init(a1);
    mpz_init(b1);
    mpz_init(one);
    mpz_set_ui(one, 1);
    mpz_set(a1, a);
    mpz_set(b1, b);
    while (mpz_sgn(b) != 0) {
        mpz_set(t, b);
        pow_mod(b, a, one, b);
        mpz_set(a, t);
    }
    mpz_set(d, a);
    mpz_set(a, a1);
    mpz_set(b, b1);
    mpz_clear(t);
    mpz_clear(a1);
    mpz_clear(b1);
    mpz_clear(one);
}

void mod_inverse(mpz_t i, mpz_t a, mpz_t n) {
    //mpz_invert(i, a, n); //MAKE YOUR OWN LATER

    mpz_t r, r1, t, t1, q, y1, y;
    mpz_init(r);
    mpz_init(r1);
    mpz_init(t);
    mpz_init(t1);
    mpz_init(q);
    mpz_init(y1);
    mpz_init(y);
    mpz_set(r, n);
    mpz_set(r1, a);
    mpz_set_ui(t, 0);
    mpz_set_ui(t1, 1);
    while (mpz_sgn(r1) != 0) {
        mpz_mdiv(q, r, r1);

        mpz_mul(y, q, r1);
        mpz_sub(y, r, y);
        mpz_set(r, r1);
        mpz_set(r1, y);

        //gmp_printf(" r  %d r1 %d  r = %Zd\n  r1 = %Zd\n",mpz_sgn(r) , mpz_sgn(r1) , r, r1);

        mpz_mul(y1, q, t1);
        mpz_sub(y1, t, y1);
        mpz_set(t, t1);
        mpz_set(t1, y1);
    }

    if (mpz_sgn(r) > 1) {
        mpz_clear(t1);
        mpz_clear(r);
        mpz_clear(r1);
        mpz_clear(t);
        mpz_set_ui(i, 0);
        return;
    }

    if (mpz_sgn(t) < 0)
        mpz_add(t, t, n);

    mpz_set(i, t);

    mpz_clear(t);
    mpz_clear(t1);
    mpz_clear(r);
    mpz_clear(r1);
}

void pow_mod(mpz_t out, mpz_t base, mpz_t exponent, mpz_t modulus) { //ISSUE WITH POWER MOD

    //mpz_powm(out, base, exponent, modulus); //FIX
    //gmp_printf("assigned out exponent %d %Zd\n",mpz_odd_p(out), out);
    mpz_t p, v, e;
    mpz_init(p);
    mpz_init(v);
    mpz_init(e);

    mpz_set_ui(v, 1); // out = 1
    mpz_set(p, base);
    mpz_set(e, exponent);
    //while (mpz_cmp_ui(exponent, 0) > 0) { //exponent > 0
    while (mpz_sgn(exponent) > 0) { //exponent > 0
        //gmp_printf("exponent %d %Zd\n",mpz_sgn(exponent), exponent);
        if (mpz_odd_p(exponent) != 0) { // exponent ODD?
            //gmp_printf("in if exponent %d %Zd\n",mpz_odd_p(exponent), exponent);
            //base = base^2 % mod
            mpz_mul(v, v, p); //base = out * base
            mpz_mod(v, v, modulus);
        }
        mpz_mul(p, p, p);
        mpz_mod(p, p, modulus);
        mpz_mdiv_ui(exponent, exponent, 2);
    }
    mpz_set(out, v);
    mpz_set(exponent, e);
    //gmp_printf("assigned out exponent %d    %Zd   %Zd\n",mpz_odd_p(out), out, base);
    mpz_clear(p);
    mpz_clear(v);
    mpz_clear(e);

    return;
}

bool is_prime(mpz_t n, uint64_t iters) {

    bool prime = true;
    mpz_t tmp_n;
    mpz_init(tmp_n);
    mpz_sub_ui(tmp_n, n, 1); // tmp_n = n - 1;

    // n - 1 = (2^s)*r
    uint64_t s = 0;
    //This finds value of s
    while (mpz_divisible_ui_p(tmp_n, 2) != 0) { // Is tmp_n divisible by 2
        mpz_tdiv_q_ui(tmp_n, tmp_n, 2); // tmp_n = tmp_n/2
        s++;
    }
    //gmp_printf("%d s\n", s);
    mpz_t r;
    mpz_init(r);
    mpz_set(r, tmp_n); // r = tmp_n

    //gmp_printf("%Zd r\n", r);
    if (mpz_divisible_ui_p(r, 2) != 0) { // r must be ODD
        prime = false;
    }
    //gmp_printf("Seg Fault\n");
    mpz_t a;
    mpz_init(a); // random value
    mpz_t y;
    mpz_init(y);
    mpz_t j;
    mpz_init(j);

    mpz_t tmp_nr;
    mpz_init(tmp_nr);
    mpz_sub_ui(tmp_nr, n, 4); // tmp_nr = n - 4
    mpz_sub_ui(tmp_n, n, 1); //tmp_n = n - 1
    mpz_t two;
    mpz_init(two);
    mpz_set_ui(two, 2); //need a number 2 in mpz

    for (uint64_t i = 1; i < iters; i++) {
        if (!prime)
            break;
        mpz_urandomm(a, state, tmp_nr); // a element of {0, 1, ..., n - 4}
        mpz_add_ui(a, a, 2); // a element of {2, 3, ..., n - 2}
        pow_mod(y, a, r, n); // y = a^r mod n
        if (mpz_cmp_ui(y, 1) != 0 && (mpz_cmp(y, tmp_n) != 0)) {
            mpz_set_ui(j, 1); // j = 1
            while ((mpz_cmp_ui(j, s - 1) <= 0) && (mpz_cmp(y, tmp_n) != 0)) { // j <= s-1 && y!= 1
                if (!prime)
                    break;

                pow_mod(y, y, two, n); // y = y^2 mod n
                if (mpz_cmp_ui(y, 1) == 0) { //y == 1
                    prime = false;
                    break;
                }
                mpz_add_ui(j, j, 1);
            }
            //mpz_sub_ui(tmp_n, n, 1); //tmp = n - 1
            if (mpz_cmp(y, tmp_n) != 0) { // y != n - 1
                prime = false;
                break;
            }
        }
    }

    // Freeing
    mpz_clear(tmp_n);
    mpz_clear(tmp_nr);
    mpz_clear(r);
    mpz_clear(a);
    mpz_clear(y);
    mpz_clear(j);
    //gmp_printf("SEGFAULT\n", r);
    return prime;
}

void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    bool has_prime = false; // Do not have a prime YET (growth mindset)
    mpz_t offset;
    mpz_init(offset);
    mpz_ui_pow_ui(offset, 2, bits - 1); // offset = 2^(bits - 1)
    mpz_t p_rand;
    mpz_init(p_rand); // pseudo random
    //gmp_randstate_t rstate;
    //gmp_randinit_mt(rstate);
    //gmp_randseed_ui(rstate, 1);

    while (!has_prime) { // while we do not have prime, look for prime
        //Always odd

        mpz_urandomb(p_rand, state, bits - 3); //p_r [0, 2^(bits-3)]
        mpz_mul_ui(p_rand, p_rand, 2); // [0, 2^(bits-2)] Only Even
        mpz_add_ui(p_rand, p_rand, 1); // [1, 2^(bits-2) + 1] Always ODD	CHECK AT MAX VALUE

        mpz_add(p, offset, p_rand); // p = offset + p_rand
        has_prime = is_prime(p, iters);
        //gmp_printf("%Zd prime %s \n", p, has_prime ? "true" : "false");
    }
    //gmp_printf("%Zd prime %s \n", p, has_prime ? "true" : "false");

    mpz_clear(offset);
    mpz_clear(p_rand);

    return;
}
