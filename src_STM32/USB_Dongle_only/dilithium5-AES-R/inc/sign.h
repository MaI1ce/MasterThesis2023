#ifndef SIGN_H
#define SIGN_H


#define CONSTRAINED_DEVICE

#include <stddef.h>
#include <stdint.h>
#include "params.h"
#include "polyvec.h"
#include "poly.h"

#ifdef STM32WB55xx
#include "randombytes.h"
#endif




#define challenge DILITHIUM_NAMESPACE(_challenge)
void challenge(poly *c, const uint8_t seed[SEEDBYTES]);

#ifdef STM32WB55xx
#define crypto_sign_keypair DILITHIUM_NAMESPACE(_keypair)
int crypto_sign_keypair(RNG_HandleTypeDef *hrng, uint8_t *pk, uint8_t *sk);

#define crypto_sign_signature DILITHIUM_NAMESPACE(_signature)
int crypto_sign_signature(RNG_HandleTypeDef *hrng, uint8_t *sig, size_t *siglen,
                          const uint8_t *m, size_t mlen,
                          const uint8_t *sk);

#define crypto_sign DILITHIUM_NAMESPACE()
int crypto_sign(RNG_HandleTypeDef *hrng, uint8_t *sm, size_t *smlen,
                const uint8_t *m, size_t mlen,
                const uint8_t *sk);
#else
#define crypto_sign_keypair DILITHIUM_NAMESPACE(_keypair)
int crypto_sign_keypair(uint8_t *pk, uint8_t *sk);

#define crypto_sign_signature DILITHIUM_NAMESPACE(_signature)
int crypto_sign_signature(uint8_t *sig, size_t *siglen,
                          const uint8_t *m, size_t mlen,
                          const uint8_t *sk);

#define crypto_sign DILITHIUM_NAMESPACE()
int crypto_sign(uint8_t *sm, size_t *smlen,
                const uint8_t *m, size_t mlen,
                const uint8_t *sk);
#endif

#define crypto_sign_verify DILITHIUM_NAMESPACE(_verify)
int crypto_sign_verify(const uint8_t *sig, size_t siglen,
                       const uint8_t *m, size_t mlen,
                       const uint8_t *pk);

#define crypto_sign_open DILITHIUM_NAMESPACE(_open)
int crypto_sign_open(uint8_t *m, size_t *mlen,
                     const uint8_t *sm, size_t smlen,
                     const uint8_t *pk);

#endif
