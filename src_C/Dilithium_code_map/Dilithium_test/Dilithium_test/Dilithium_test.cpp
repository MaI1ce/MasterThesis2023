// Dilithium_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS
//#define STATIC_KEYS
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

#ifdef STATIC_KEYS
#include "dilithium5-AES-R/dilithium_keys.h"
#endif

#ifdef DEBUG_LOG
#include "debug_log.h"
#endif

#define MLEN 59
#define NTESTS 1

//#define TEST_DILITHIUM
//#define VERIFY_TEST
//#define KEY_GENERATOR_TEST
//#define SIGN_TEST
#define Y_GENERATION

    int main()
    {
        log_debug_log_init("dilithium_log.log");

#ifdef Y_GENERATION
        FILE* fptr_std;
        FILE* fptr_ntt;

        uint8_t rhoprime[SEEDBYTES] = {0xAA, 0xAA, 0xAA, 0xAA ,0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, };
        polyvecl y;
        polyvecl z;
        int nonce = 0;

        fopen_s(&fptr_std, "./std_y.txt", "w");
        fopen_s(&fptr_ntt, "./ntt_y.txt", "w");

        fprintf(fptr_std, "{\n");
        fprintf(fptr_ntt, "{\n");

        for (int i = 0; i < 10; i++) {
            polyvecl_uniform_gamma1(&y, rhoprime, nonce++);
            z = y;
            polyvecl_ntt(&z);
            fprintf(fptr_std, "\t{\n");
            fprintf(fptr_ntt, "\t{\n");
            for (int j = 0; j < L; j++) {
                fprintf(fptr_std, "\t\t{");
                fprintf(fptr_ntt, "\t\t{");
                for (int n = 0; n < N; n++) {
                    fprintf(fptr_std, "0x%02x, ", y.vec[j].coeffs[n]);
                    fprintf(fptr_ntt, "0x%02x, ", z.vec[j].coeffs[n]);
                }
                fprintf(fptr_std, "},\n");
                fprintf(fptr_ntt, "},\n");
            }
            fprintf(fptr_std, "\t},\n");
            fprintf(fptr_ntt, "\t},\n");

        }
        fprintf(fptr_std, "}\n");
        fprintf(fptr_ntt, "}\n");



        fclose(fptr_std);
        fclose(fptr_ntt);
#endif

#ifdef KEY_GENERATOR_TEST // TEST - OK keys are identical for 5 tests
        FILE* pk_fptr;
        uint8_t pk[CRYPTO_PUBLICKEYBYTES];
        uint8_t sk[(2 * SEEDBYTES + CRHBYTES \
            + L * POLYETA_PACKEDBYTES \
            + K * POLYETA_PACKEDBYTES \
            + K * POLYT0_PACKEDBYTES)];
        uint8_t sk2[(3 * SEEDBYTES + CRHBYTES \
            + K * POLYT0_PACKEDBYTES)];

        unsigned int i, j;
        int ret;
        size_t mlen, smlen;
        uint8_t m[MLEN] = { 0x55 };
        uint8_t sm[MLEN + CRYPTO_BYTES];
        uint8_t m2[MLEN + CRYPTO_BYTES];

        char pk_file_name[80] = { 0 };

        keygen(pk, sk, sk2);

        sprintf(pk_file_name, "./dilithium5-AES-R/dilithium_keys.c");
        fopen_s(&pk_fptr, pk_file_name, "w");

        fprintf(pk_fptr, "#include \"dilithium_keys.h\"\n\n");
        fprintf(pk_fptr, "uint8_t pk[CRYPTO_PUBLICKEYBYTES] = {");

        for (int i = 0; i < CRYPTO_PUBLICKEYBYTES; i++) {
            if (i % 8 == 0)
                fprintf(pk_fptr, "\n\t");
            fprintf(pk_fptr, "0x%02x, ", pk[i]);
        }

        fprintf(pk_fptr, "\n};\n\n");
        fprintf(pk_fptr, "uint8_t sk[CRYPTO_SECRETKEYBYTES] = {\n");
        fprintf(pk_fptr, "#ifndef CONSTRAINED_DEVICE\n");

        for (int i = 0; i < sizeof(sk); i++) {
            if (i % 8 == 0)
                fprintf(pk_fptr, "\n\t");
            fprintf(pk_fptr, "0x%02x, ", sk[i]);
        }

        fprintf(pk_fptr, "\n#else\n");

        for (int i = 0; i < sizeof(sk2); i++) {
            if (i % 8 == 0)
                fprintf(pk_fptr, "\n\t");
            fprintf(pk_fptr, "0x%02x, ", sk2[i]);
        }

        fprintf(pk_fptr, "\n#endif\n");
        fprintf(pk_fptr, "};\n\n");

        fclose(pk_fptr);

        crypto_sign(sm, &smlen, m, MLEN, sk);

        ret = crypto_sign_open(m2, &mlen, sm, smlen, pk);


        if (ret) {
            fprintf(stderr, "Verification 1 failed, err_code = %d\n", ret);
            return -1;
        }
        else {
            fprintf(stderr, "Verification 1 - OK\n");
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
        printf("TEST - OK\n");
     
#endif//KEY_GENERATOR_TEST

#ifdef VERIFY_TEST //ndef CONST_RAND_SEED
        uint8_t pk[CRYPTO_PUBLICKEYBYTES];
        uint8_t sk[CRYPTO_SECRETKEYBYTES];
        size_t mlen, smlen;
        uint8_t m[MLEN] = { 0x55 };
        uint8_t sm[MLEN + CRYPTO_BYTES];
        uint8_t m2[MLEN + CRYPTO_BYTES];

        for (int i = 0; i < NTESTS; i++)
        {
            crypto_sign_keypair(pk, sk);
            crypto_sign_signature(sm, &smlen, sm + CRYPTO_BYTES, MLEN, sk);

            int err_code = 0;

            err_code = crypto_sign_verify(sm, smlen, sm + CRYPTO_BYTES, MLEN, pk);
            if (err_code) {
                printf("STANDARD VERIFY FAILED - ERROR CODE = %d\n", err_code);
            }
            err_code = crypto_sign_verify2(sm, smlen, sm + CRYPTO_BYTES, MLEN, pk);
            if (err_code) {
                printf("NEW VERIFY FAILED - ERROR CODE = %d\n", err_code);
            }
            printf("\n");
        }

#endif //VERIFY_TEST

#ifdef SIGN_TEST
        uint8_t pk[CRYPTO_PUBLICKEYBYTES];
        uint8_t sk[CRYPTO_SECRETKEYBYTES];
        size_t mlen, smlen;
        uint8_t m[MLEN] = { 0x55 };
        uint8_t sm[MLEN + CRYPTO_BYTES];
        uint8_t m2[MLEN + CRYPTO_BYTES];

        for (int i = 0; i < NTESTS; i++)
        {
            crypto_sign_keypair2(pk, sk);
            crypto_sign_signature2(sm, &smlen, sm + CRYPTO_BYTES, MLEN, sk);

            int err_code = 0;

            err_code = crypto_sign_verify(sm, smlen, sm + CRYPTO_BYTES, MLEN, pk);
            if (err_code) {
                printf("STANDARD VERIFY FAILED - ERROR CODE = %d\n", err_code);
            }
            err_code = crypto_sign_verify2(sm, smlen, sm + CRYPTO_BYTES, MLEN, pk);
            if (err_code) {
                printf("NEW VERIFY FAILED - ERROR CODE = %d\n", err_code);
            }
            printf("\n");

        }

#endif //SIGN_TEST

#ifdef TEST_DILITHIUM
//#define SAVE_KEYS
        unsigned int i, j;
        int ret;
        size_t mlen, smlen;
        uint8_t m[MLEN] = { 0x55 };
        uint8_t sm[MLEN + CRYPTO_BYTES];
        uint8_t m2[MLEN + CRYPTO_BYTES];
#ifndef STATIC_KEYS
        uint8_t pk[CRYPTO_PUBLICKEYBYTES];
        uint8_t sk[CRYPTO_SECRETKEYBYTES];
#endif


        for (i = 0; i < NTESTS; ++i) {

            //randombytes(m, MLEN);

#ifndef STATIC_KEYS
            crypto_sign_keypair(pk, sk);
#endif
            crypto_sign(sm, &smlen, m, MLEN, sk);

            ret = crypto_sign_open(m2, &mlen, sm, smlen, pk);


            if (ret) {
                fprintf(stderr, "Verification 1 failed, err_code = %d\n", ret);
                return -1;
            }
            else {
                fprintf(stderr, "Verification 1 - OK\n");
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
            /*
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
             */
        }
        printf("TEST - OK\n");
        printf("CRYPTO_PUBLICKEYBYTES = %d\n", CRYPTO_PUBLICKEYBYTES);
        printf("CRYPTO_SECRETKEYBYTES = %d\n", CRYPTO_SECRETKEYBYTES);
        printf("CRYPTO_BYTES = %d\n", CRYPTO_BYTES);

#endif
        log_debug_deinit();
        return 0;

    }
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
