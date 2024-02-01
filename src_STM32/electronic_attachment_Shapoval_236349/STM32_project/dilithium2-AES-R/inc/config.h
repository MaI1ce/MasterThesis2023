#ifndef CONFIG_H
#define CONFIG_H

#define DILITHIUM_MODE 2
#define DILITHIUM_USE_AES
//#define DILITHIUM_RANDOMIZED_SIGNING

#define CRYPTO_ALGNAME "Dilithium2-AES-R"
#define DILITHIUM_NAMESPACE(s) pqcrystals_dilithium2aes_r_ref##s

//#define HARDWARE_AES
//#define CONSTRAINED_DEVICE
//#define DEBUG_LOG
//#define TIME_BENCHMARK

#ifdef TIME_BENCHMARK
#define KEYS_SMUL_TIMESTAMP 0
#define SIGN_YMUL_TIMESTAMP 1
#define SIGN_Z_COMPUTE_TIMESTAMP 2
#define SIGN_H_COMPUTE_TIMESTAMP 3
#define VERIFY_RECONSTRUC_W1_TIMESTAMP 4
#define FULL_SIGN_TIMESTAMP 5
#define FULL_VERIFY_TIMESTAMP 6
#define FULL_KEYGEN 7
#endif

#endif
