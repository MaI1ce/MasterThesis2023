#ifndef PACKING_H
#define PACKING_H

#include <stdint.h>
#include "params.h"
#include "polyvec.h"

#define pack_pk DILITHIUM_NAMESPACE(_pack_pk)
void pack_pk(uint8_t pk[CRYPTO_PUBLICKEYBYTES],
             const uint8_t rho[SEEDBYTES], const polyveck *t1);

#define pack_sk DILITHIUM_NAMESPACE(_pack_sk)
void pack_sk(uint8_t sk[CRYPTO_SECRETKEYBYTES],
             const uint8_t rho[SEEDBYTES],
             const uint8_t tr[CRHBYTES],
             const uint8_t key[SEEDBYTES],
             const polyveck *t0,
             const polyvecl *s1,
             const polyveck *s2);

#define pack_sk_r DILITHIUM_NAMESPACE(_pack_sk_r)
void pack_sk_r(uint8_t sk[CRYPTO_SECRETKEYBYTES],
             const uint8_t rho[SEEDBYTES],
			 const uint8_t rhoprime[SEEDBYTES],
             const uint8_t tr[CRHBYTES],
             const uint8_t key[SEEDBYTES],
             const polyveck *t0);

#define pack_sig DILITHIUM_NAMESPACE(_pack_sig)
void pack_sig(uint8_t sig[CRYPTO_BYTES],
              const uint8_t c[SEEDBYTES], const polyvecl *z, const polyveck *h);

#define unpack_pk DILITHIUM_NAMESPACE(_unpack_pk)
void unpack_pk(uint8_t rho[SEEDBYTES], polyveck *t1,
               const uint8_t pk[CRYPTO_PUBLICKEYBYTES]);

#define unpack_sk DILITHIUM_NAMESPACE(_unpack_sk)
void unpack_sk(uint8_t rho[SEEDBYTES],
               uint8_t tr[CRHBYTES],
               uint8_t key[SEEDBYTES],
               polyveck *t0,
               polyvecl *s1,
               polyveck *s2,
               const uint8_t sk[CRYPTO_SECRETKEYBYTES]);

#define unpack_sk_r DILITHIUM_NAMESPACE(_unpack_sk_r)
void unpack_sk_r(uint8_t rho[SEEDBYTES],
			   uint8_t rhoprime[SEEDBYTES],
               uint8_t tr[CRHBYTES],
               uint8_t key[SEEDBYTES],
               polyveck *t0,
               const uint8_t sk[CRYPTO_SECRETKEYBYTES]);

#define unpack_sig DILITHIUM_NAMESPACE(_unpack_sig)
int unpack_sig(uint8_t c[SEEDBYTES], polyvecl *z, polyveck *h,
               const uint8_t sig[CRYPTO_BYTES]);


////////////////////////////////////////////

int verify_sig_z_h_malform(uint8_t h_buf[N_*K], const uint8_t sig[CRYPTO_BYTES]);

void unpack_pk_t1(poly *t1_i, uint32_t i, const uint8_t pk[CRYPTO_PUBLICKEYBYTES]);

void unpack_pk_rho(uint8_t rho[SEEDBYTES], const uint8_t pk[CRYPTO_PUBLICKEYBYTES]);

void unpack_sig_z(poly *z_i, uint32_t i, const uint8_t sig[CRYPTO_BYTES]);

void unpack_sig_c(uint8_t c[SEEDBYTES], const uint8_t sig[CRYPTO_BYTES]);

#endif
