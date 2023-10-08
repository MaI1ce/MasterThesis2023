// Dilithium_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

extern "C" {

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dilithium5-AES-R/randombytes.h"
#include "dilithium5-AES-R/fips202.h"
#include "dilithium5-AES-R/params.h"
#include "dilithium5-AES-R/sign.h"
#include "dilithium5-AES-R/poly.h"
#include "dilithium5-AES-R/polyvec.h"
#include "dilithium5-AES-R/packing.h"
#include <stddef.h>


static void poly_naivemul(poly* c, const poly* a, const poly* b) {
    unsigned int i, j;
    int32_t r[2 * N] = { 0 };

    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            r[i + j] = (r[i + j] + (int64_t)a->coeffs[i] * b->coeffs[j]) % Q;

    for (i = N; i < 2 * N; i++)
        r[i - N] = (r[i - N] - r[i]) % Q;

    for (i = 0; i < N; i++)
        c->coeffs[i] = r[i];
}

}

//#define TEST_VECTORS
#define TEST_DILITHIUM
//#define TEST_MUL


int main()
{

#ifdef TEST_VECTORS

#define MLEN 32
#define NVECTORS 1

    unsigned int i, j, k, l;
    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[CRYPTO_SECRETKEYBYTES];
    uint8_t sig[CRYPTO_BYTES];
    uint8_t m[MLEN] = { 0 };
    //  __attribute__((aligned(32)))
    uint8_t seed[CRHBYTES];
    uint8_t buf[CRYPTO_SECRETKEYBYTES];
    size_t siglen;
    poly c, tmp;
    polyvecl s, y, mat[K];
    polyveck w, w1, w0, t1, t0, h;
    int32_t u;

    for (i = 0; i < NVECTORS; ++i) {
        printf("count = %u\n", i);

        randombytes(m, MLEN);
        printf("m = ");
        for (j = 0; j < MLEN; ++j)
            printf("%02x", m[j]);
        printf("\n");

        crypto_sign_keypair(pk, sk);
        shake256(buf, 32, pk, CRYPTO_PUBLICKEYBYTES);
        printf("pk = ");
        for (j = 0; j < 32; ++j)
            printf("%02x", buf[j]);
        printf("\n");
        shake256(buf, 32, sk, CRYPTO_SECRETKEYBYTES);
        printf("sk = ");
        for (j = 0; j < 32; ++j)
            printf("%02x", buf[j]);
        printf("\n");

        crypto_sign_signature(sig, &siglen, m, MLEN, sk);
        shake256(buf, 32, sig, CRYPTO_BYTES);
        printf("sig = ");
        for (j = 0; j < 32; ++j)
            printf("%02x", buf[j]);
        printf("\n");

        if (crypto_sign_verify(sig, siglen, m, MLEN, pk))
            fprintf(stderr, "Signature verification failed!\n");

        randombytes(seed, sizeof(seed));
        printf("seed = ");
        for (j = 0; j < sizeof(seed); ++j)
            printf("%02X", seed[j]);
        printf("\n");

        polyvec_matrix_expand(mat, seed);
        /*printf("A = ([");
        for (j = 0; j < K; ++j) {
            for (k = 0; k < L; ++k) {
                for (l = 0; l < N; ++l) {
                    printf("%8d", mat[j].vec[k].coeffs[l]);
                    if (l < N - 1) printf(", ");
                    else if (k < L - 1) printf("], [");
                    else if (j < K - 1) printf("];\n     [");
                    else printf("])\n");
                }
            }
        }*/

        polyvecl_uniform_eta(&s, seed, 0);

        polyeta_pack(buf, &s.vec[0]);
        polyeta_unpack(&tmp, buf);
        for (j = 0; j < N; ++j)
            if (tmp.coeffs[j] != s.vec[0].coeffs[j])
                fprintf(stderr, "ERROR in polyeta_(un)pack!\n");

        polyvecl_reduce(&s);
        if (polyvecl_chknorm(&s, ETA + 1))
            fprintf(stderr, "ERROR in polyvecl_chknorm(&s ,ETA+1)!\n");
        /*
        printf("s = ([");
        for (j = 0; j < L; ++j) {
            for (k = 0; k < N; ++k) {
                u = s.vec[j].coeffs[k];
                printf("%3d", u);
                if (k < N - 1) printf(", ");
                else if (j < L - 1) printf("],\n     [");
                else printf("])\n");
            }
        }*/

        polyvecl_uniform_gamma1(&y, seed, 0);

        polyz_pack(buf, &y.vec[0]);
        polyz_unpack(&tmp, buf);
        for (j = 0; j < N; ++j)
            if (tmp.coeffs[j] != y.vec[0].coeffs[j])
                fprintf(stderr, "ERROR in polyz_(un)pack!\n");

        if (polyvecl_chknorm(&y, GAMMA1 + 1))
            fprintf(stderr, "ERROR in polyvecl_chknorm(&y, GAMMA1)!\n");
        /*
        printf("y = ([");
        for (j = 0; j < L; ++j) {
            for (k = 0; k < N; ++k) {
                u = y.vec[j].coeffs[k];
                printf("%8d", u);
                if (k < N - 1) printf(", ");
                else if (j < L - 1) printf("],\n     [");
                else printf("])\n");
            }
        }*/

        polyvecl_ntt(&y);
        polyvec_matrix_pointwise_montgomery(&w, mat, &y);
        polyveck_reduce(&w);
        polyveck_invntt_tomont(&w);
        polyveck_caddq(&w);
        polyveck_decompose(&w1, &w0, &w);

        for (j = 0; j < N; ++j) {
            tmp.coeffs[j] = w1.vec[0].coeffs[j] * 2 * GAMMA2 + w0.vec[0].coeffs[j];
            if (tmp.coeffs[j] < 0) tmp.coeffs[j] += Q;
            if (tmp.coeffs[j] != w.vec[0].coeffs[j])
                fprintf(stderr, "ERROR in poly_decompose!\n");
        }

        polyw1_pack(buf, &w1.vec[0]);
#if GAMMA2 == (Q-1)/32
        for (j = 0; j < N / 2; ++j) {
            tmp.coeffs[2 * j + 0] = buf[j] & 0xF;
            tmp.coeffs[2 * j + 1] = buf[j] >> 4;
            if (tmp.coeffs[2 * j + 0] != w1.vec[0].coeffs[2 * j + 0]
                || tmp.coeffs[2 * j + 1] != w1.vec[0].coeffs[2 * j + 1])
                fprintf(stderr, "ERROR in polyw1_pack!\n");
        }
#endif

#if GAMMA2 == (Q-1)/32
        if (polyveck_chknorm(&w1, 16))
            fprintf(stderr, "ERROR in polyveck_chknorm(&w1, 16)!\n");
#elif GAMMA2 == (Q-1)/88
        if (polyveck_chknorm(&w1, 44))
            fprintf(stderr, "ERROR in polyveck_chknorm(&w1, 4)!\n");
#endif
        if (polyveck_chknorm(&w0, GAMMA2 + 1))
            fprintf(stderr, "ERROR in polyveck_chknorm(&w0, GAMMA2+1)!\n");
        /*
        printf("w1 = ([");
        for (j = 0; j < K; ++j) {
            for (k = 0; k < N; ++k) {
                printf("%2d", w1.vec[j].coeffs[k]);
                if (k < N - 1) printf(", ");
                else if (j < K - 1) printf("],\n      [");
                else printf("])\n");
            }
        }
        printf("w0 = ([");
        for (j = 0; j < K; ++j) {
            for (k = 0; k < N; ++k) {
                u = w0.vec[j].coeffs[k];
                printf("%8d", u);
                if (k < N - 1) printf(", ");
                else if (j < K - 1) printf("],\n      [");
                else printf("])\n");
            }
        }
        */
        polyveck_power2round(&t1, &t0, &w);

        for (j = 0; j < N; ++j) {
            tmp.coeffs[j] = (t1.vec[0].coeffs[j] << D) + t0.vec[0].coeffs[j];
            if (tmp.coeffs[j] != w.vec[0].coeffs[j])
                fprintf(stderr, "ERROR in poly_power2round!\n");
        }

        polyt1_pack(buf, &t1.vec[0]);
        polyt1_unpack(&tmp, buf);
        for (j = 0; j < N; ++j) {
            if (tmp.coeffs[j] != t1.vec[0].coeffs[j])
                fprintf(stderr, "ERROR in polyt1_(un)pack!\n");
        }
        polyt0_pack(buf, &t0.vec[0]);
        polyt0_unpack(&tmp, buf);
        for (j = 0; j < N; ++j) {
            if (tmp.coeffs[j] != t0.vec[0].coeffs[j])
                fprintf(stderr, "ERROR in polyt0_(un)pack!\n");
        }

        if (polyveck_chknorm(&t1, 1024))
            fprintf(stderr, "ERROR in polyveck_chknorm(&t1, 1024)!\n");
        if (polyveck_chknorm(&t0, (1U << (D - 1)) + 1))
            fprintf(stderr, "ERROR in polyveck_chknorm(&t0, (1 << (D-1)) + 1)!\n");
        /*
        printf("t1 = ([");
        for (j = 0; j < K; ++j) {
            for (k = 0; k < N; ++k) {
                printf("%3d", t1.vec[j].coeffs[k]);
                if (k < N - 1) printf(", ");
                else if (j < K - 1) printf("],\n      [");
                else printf("])\n");
            }
        }
        printf("t0 = ([");
        for (j = 0; j < K; ++j) {
            for (k = 0; k < N; ++k) {
                u = t0.vec[j].coeffs[k];
                printf("%5d", u);
                if (k < N - 1) printf(", ");
                else if (j < K - 1) printf("],\n      [");
                else printf("])\n");
            }
        }
        */
        poly_challenge(&c, seed);
        /*
        printf("c = [");
        for (j = 0; j < N; ++j) {
            u = c.coeffs[j];
            printf("%2d", u);
            if (j < N - 1) printf(", ");
            else printf("]\n");
        }*/

        polyveck_make_hint(&h, &w0, &w1);
        pack_sig(buf, seed, &y, &h);
        unpack_sig(seed, &y, &w, buf);
        if (memcmp(&h, &w, sizeof(h)))
            fprintf(stderr, "ERROR in (un)pack_sig!\n");

        printf("\n");
    }
#endif

#ifdef TEST_DILITHIUM
#define MLEN 59
#define NTESTS 1

    unsigned int i, j;
    int ret;
    size_t mlen, smlen;
    uint8_t m[MLEN] = { 0 };
    uint8_t sm[MLEN + CRYPTO_BYTES];
    uint8_t m2[MLEN + CRYPTO_BYTES];
    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[CRYPTO_SECRETKEYBYTES];

    for (i = 0; i < NTESTS; ++i) {
        randombytes(m, MLEN);

        crypto_sign_keypair(pk, sk);
        crypto_sign(sm, &smlen, m, MLEN, sk);
        ret = crypto_sign_open(m2, &mlen, sm, smlen, pk);

        if (ret) {
            fprintf(stderr, "Verification failed\n");
            return -1;
        }

        if (mlen != MLEN) {
            fprintf(stderr, "Message lengths don't match\n");
            return -1;
        }

        for (j = 0; j < mlen; ++j) {
            if (m[j] != m2[j]) {
                fprintf(stderr, "Messages don't match\n");
                return -1;
            }
        }

        randombytes((uint8_t*)&j, sizeof(j));
        do {
            randombytes(m2, 1);
        } while (!m2[0]);
        sm[j % CRYPTO_BYTES] += m2[0];
        ret = crypto_sign_open(m2, &mlen, sm, smlen, pk);
        if (!ret) {
            fprintf(stderr, "Trivial forgeries possible\n");
            return -1;
        }
    }

    printf("CRYPTO_PUBLICKEYBYTES = %d\n", CRYPTO_PUBLICKEYBYTES);
    printf("CRYPTO_SECRETKEYBYTES = %d\n", CRYPTO_SECRETKEYBYTES);
    printf("CRYPTO_BYTES = %d\n", CRYPTO_BYTES);

#endif

#ifdef TEST_MUL
#define NTESTS 1


    unsigned int i, j;
    uint8_t seed[SEEDBYTES];
    uint16_t nonce = 0;
    poly a, b, c, d;

    randombytes(seed, sizeof(seed));
    for (i = 0; i < NTESTS; ++i) {
        poly_uniform(&a, seed, nonce++);
        poly_uniform(&b, seed, nonce++);

        c = a;
        poly_ntt(&c);
        for (j = 0; j < N; ++j)
            c.coeffs[j] = (int64_t)c.coeffs[j] * -114592 % Q;
        poly_invntt_tomont(&c);
        for (j = 0; j < N; ++j) {
            if ((c.coeffs[j] - a.coeffs[j]) % Q)
                fprintf(stderr, "ERROR in ntt/invntt: c[%d] = %d != %d\n",
                    j, c.coeffs[j] % Q, a.coeffs[j]);
        }

        poly_naivemul(&c, &a, &b);
        poly_ntt(&a);
        poly_ntt(&b);
        poly_pointwise_montgomery(&d, &a, &b);
        poly_invntt_tomont(&d);

        for (j = 0; j < N; ++j) {
            if ((d.coeffs[j] - c.coeffs[j]) % Q)
                fprintf(stderr, "ERROR in multiplication: d[%d] = %d != %d\n",
                    j, d.coeffs[j], c.coeffs[j]);
        }
    }
#endif

    return 0;
   
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
