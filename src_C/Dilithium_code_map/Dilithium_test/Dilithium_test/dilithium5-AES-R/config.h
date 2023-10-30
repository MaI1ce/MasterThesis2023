#ifndef CONFIG_H
#define CONFIG_H

#define DILITHIUM_MODE 2
#define DILITHIUM_USE_AES
//#define DILITHIUM_RANDOMIZED_SIGNING

#define CRYPTO_ALGNAME "Dilithium5-AES-R"
#define DILITHIUM_NAMESPACE(s) pqcrystals_dilithium5aes_r_ref##s

//#define CONSTRAINED_DEVICE
//#define CONST_RAND_SEED
//#define SECRET_KEY_TEST
#define DEBUG_LOG

#endif
