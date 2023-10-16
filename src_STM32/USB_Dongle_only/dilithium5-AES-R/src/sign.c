#include <stdint.h>
#include "params.h"
#include "sign.h"
#include "packing.h"
#include "polyvec.h"
#include "poly.h"
#include "randombytes.h"
#include "symmetric.h"
#include "fips202.h"


/*************************************************
* Name:        crypto_sign_keypair
*
* Description: Generates public and private key.
*
* Arguments:   - uint8_t *pk: pointer to output public key (allocated
*                             array of CRYPTO_PUBLICKEYBYTES bytes)
*              - uint8_t *sk: pointer to output private key (allocated
*                             array of CRYPTO_SECRETKEYBYTES bytes)
*
* Returns 0 (success)
**************************************************/
#ifdef STM32WB55xx
int crypto_sign_keypair(RNG_HandleTypeDef *hrng, uint8_t *pk, uint8_t *sk)
#else
int crypto_sign_keypair(uint8_t *pk, uint8_t *sk)
#endif
{
	uint8_t seedbuf[3*SEEDBYTES];
	uint8_t tr[CRHBYTES];
	const uint8_t *rho, *rhoprime, *key;

#ifdef CONSTRAINED_DEVICE
	polyveck t1, t0;
#else
	polyvecl mat[K];
	polyvecl s1, s1hat;
	polyveck s2, t1, t0;

	randombytes(seedbuf, SEEDBYTES);
#endif
  /* Get randomness for rho, rhoprime and key */
#ifdef STM32WB55xx
	randombytes(hrng, seedbuf, SEEDBYTES);
#else
	randombytes(seedbuf, SEEDBYTES);
#endif
	shake256(seedbuf, 3*SEEDBYTES, seedbuf, SEEDBYTES);
	rho = seedbuf;
	rhoprime = seedbuf + SEEDBYTES;
	key = seedbuf + 2*SEEDBYTES;

#ifdef CONSTRAINED_DEVICE
	polyvec_matrix_poly_smul_montgomery(&t1, rho, rhoprime);

#else
	/* Expand matrix */
	polyvec_matrix_expand(mat, rho);

	/* Sample short vectors s1 and s2 */
	polyvecl_uniform_eta(&s1, rhoprime, 0);
	polyveck_uniform_eta(&s2, rhoprime, L);

	/* Matrix-vector multiplication */
	s1hat = s1;
	polyvecl_ntt(&s1hat);
	polyvec_matrix_pointwise_montgomery(&t1, mat, &s1hat);
	polyveck_reduce(&t1);
	polyveck_invntt_tomont(&t1);

	// Add error vector s2
	polyveck_add(&t1, &t1, &s2);
#endif

  /* Extract t1 and write public key */
  polyveck_caddq(&t1);
  polyveck_power2round(&t1, &t0, &t1);
  pack_pk(pk, rho, &t1);

  /* Compute CRH(rho, t1) and write secret key */
  crh(tr, pk, CRYPTO_PUBLICKEYBYTES);
#ifdef CONSTRAINED_DEVICE
  pack_sk_r(sk, rho, rhoprime, tr, key, &t0);
#else
  pack_sk(sk, rho, tr, key, &t0, &s1, &s2);
#endif
  return 0;
}

/*************************************************
* Name:        crypto_sign_signature
*
* Description: Computes signature.
*
* Arguments:   - uint8_t *sig:   pointer to output signature (of length CRYPTO_BYTES)
*              - size_t *siglen: pointer to output length of signature
*              - uint8_t *m:     pointer to message to be signed
*              - size_t mlen:    length of message
*              - uint8_t *sk:    pointer to bit-packed secret key
*
* Returns 0 (success)
**************************************************/
#ifdef STM32WB55xx
int crypto_sign_signature(RNG_HandleTypeDef *hrng, uint8_t *sig,
                          size_t *siglen,
                          const uint8_t *m,
                          size_t mlen,
                          const uint8_t *sk)
#else
int crypto_sign_signature(uint8_t *sig,
                          size_t *siglen,
                          const uint8_t *m,
                          size_t mlen,
                          const uint8_t *sk)
#endif
{

#ifdef CONSTRAINED_DEVICE
  unsigned int n;
  uint8_t seedbuf[3*SEEDBYTES + 3*CRHBYTES];
  uint8_t *rho, *s_rhoprime, *tr, *key, *mu, *y_rhoprime;
  uint16_t nonce = 0;
  polyvecl z;
  polyveck t0, w1, w0, h;
  poly cp;
  keccak_state state;

  rho = seedbuf;
  s_rhoprime = rho + SEEDBYTES;
  tr = s_rhoprime + SEEDBYTES;
  key = tr + CRHBYTES;
  mu = key + SEEDBYTES;
  y_rhoprime = mu + CRHBYTES;
  unpack_sk_r(rho, s_rhoprime, tr, key, &t0, sk);
#else
  unsigned int n;
  uint8_t seedbuf[2*SEEDBYTES + 3*CRHBYTES];
  uint8_t *rho, *tr, *key, *mu, *rhoprime;
  uint16_t nonce = 0;
  polyvecl mat[K], s1, y, z;
  polyveck t0, s2, w1, w0, h;
  poly cp;
  keccak_state state;

  rho = seedbuf;
  tr = rho + SEEDBYTES;
  key = tr + CRHBYTES;
  mu = key + SEEDBYTES;
  rhoprime = mu + CRHBYTES;
  unpack_sk(rho, tr, key, &t0, &s1, &s2, sk);
#endif


  /* Compute CRH(tr, msg) */
  shake256_init(&state);
  shake256_absorb(&state, tr, CRHBYTES);
  shake256_absorb(&state, m, mlen);
  shake256_finalize(&state);
  shake256_squeeze(mu, CRHBYTES, &state);

#if defined(CONSTRAINED_DEVICE) && defined(DILITHIUM_RANDOMIZED_SIGNING)
#ifdef STM32WB55xx
  randombytes(hrng, y_rhoprime, CRHBYTES);
#else
  randombytes(y_rhoprime, CRHBYTES);
#endif //STM32WB55xx
#elif defined(CONSTRAINED_DEVICE) && !defined(DILITHIUM_RANDOMIZED_SIGNING)
  crh(y_rhoprime, key, SEEDBYTES + CRHBYTES);
#elif !defined(CONSTRAINED_DEVICE) && defined(DILITHIUM_RANDOMIZED_SIGNING)
#ifdef STM32WB55xx
  randombytes(hrng, rhoprime, CRHBYTES);
#else
  randombytes(rhoprime, CRHBYTES);
#endif //STM32WB55xx
#elif !defined(CONSTRAINED_DEVICE) && !defined(DILITHIUM_RANDOMIZED_SIGNING)
  crh(rhoprime, key, SEEDBYTES + CRHBYTES);
#endif


#ifndef CONSTRAINED_DEVICE
  /* Expand matrix and transform vectors */
  polyvec_matrix_expand(mat, rho);
  polyvecl_ntt(&s1);
  polyveck_ntt(&s2);
#endif
  polyveck_ntt(&t0);

rej:

#ifndef CONSTRAINED_DEVICE
  //Sample intermediate vector y
  polyvecl_uniform_gamma1(&y, y_rhoprime, nonce++);
  z = y;
  polyvecl_ntt(&z);

  // Matrix-vector multiplication
  polyvec_matrix_pointwise_montgomery(&w1, mat, &z);
  polyveck_reduce(&w1);
  polyveck_invntt_tomont(&w1);
#else
  polyvec_matrix_poly_ymul_montgomery(&w1, rho, y_rhoprime, nonce++);
#endif

  /* Decompose w and call the random oracle */
  polyveck_caddq(&w1);
  polyveck_decompose(&w1, &w0, &w1);
  polyveck_pack_w1(sig, &w1);

  shake256_init(&state);
  shake256_absorb(&state, mu, CRHBYTES);
  shake256_absorb(&state, sig, K*POLYW1_PACKEDBYTES);
  shake256_finalize(&state);
  shake256_squeeze(sig, SEEDBYTES, &state);
  poly_challenge(&cp, sig);
  poly_ntt(&cp);

#ifndef CONSTRAINED_DEVICE
  /* Compute z, reject if it reveals secret */
  polyvecl_pointwise_poly_montgomery(&z, &cp, &s1);
  polyvecl_invntt_tomont(&z);
  polyvecl_add(&z, &z, &y);
  polyvecl_reduce(&z);

  /* Check that subtracting cs2 does not change high bits of w and low bits
   * do not reveal secret information */
  polyveck_pointwise_poly_montgomery(&h, &cp, &s2);
  polyveck_invntt_tomont(&h);


#else
  /* Compute z, reject if it reveals secret */
  polyvec_compute_z_montgomery(&z, &cp, s_rhoprime, y_rhoprime, nonce);

  if(polyvecl_chknorm(&z, GAMMA1 - BETA))
    goto rej;

  /* Check that subtracting cs2 does not change high bits of w and low bits
   * do not reveal secret information */
  polyvec_compute_h_montgomery(&h, &cp, s_rhoprime);
#endif

  polyveck_sub(&w0, &w0, &h);
  polyveck_reduce(&w0);
  if(polyveck_chknorm(&w0, GAMMA2 - BETA))
    goto rej;

  /* Compute hints for w1 */
  polyveck_pointwise_poly_montgomery(&h, &cp, &t0);
  polyveck_invntt_tomont(&h);
  polyveck_reduce(&h);
  if(polyveck_chknorm(&h, GAMMA2))
    goto rej;

  polyveck_add(&w0, &w0, &h);
  polyveck_caddq(&w0);
  n = polyveck_make_hint(&h, &w0, &w1);
  if(n > OMEGA)
    goto rej;

  /* Write signature */
  pack_sig(sig, sig, &z, &h);
  *siglen = CRYPTO_BYTES;
  return 0;
}

/*************************************************
* Name:        crypto_sign
*
* Description: Compute signed message.
*
* Arguments:   - uint8_t *sm: pointer to output signed message (allocated
*                             array with CRYPTO_BYTES + mlen bytes),
*                             can be equal to m
*              - size_t *smlen: pointer to output length of signed
*                               message
*              - const uint8_t *m: pointer to message to be signed
*              - size_t mlen: length of message
*              - const uint8_t *sk: pointer to bit-packed secret key
*
* Returns 0 (success)
**************************************************/
#ifdef STM32WB55xx
int crypto_sign(RNG_HandleTypeDef *hrng, uint8_t *sm,
                size_t *smlen,
                const uint8_t *m,
                size_t mlen,
                const uint8_t *sk)
#else
int crypto_sign(uint8_t *sm,
                size_t *smlen,
                const uint8_t *m,
                size_t mlen,
                const uint8_t *sk)
#endif
{
  size_t i;

  for(i = 0; i < mlen; ++i)
    sm[CRYPTO_BYTES + mlen - 1 - i] = m[mlen - 1 - i];
#ifdef STM32WB55xx
  crypto_sign_signature(hrng, sm, smlen, sm + CRYPTO_BYTES, mlen, sk);
#else
  crypto_sign_signature(sm, smlen, sm + CRYPTO_BYTES, mlen, sk);
#endif
  *smlen += mlen;
  return 0;
}

/*************************************************
* Name:        crypto_sign_verify
*
* Description: Verifies signature.
*
* Arguments:   - uint8_t *m: pointer to input signature
*              - size_t siglen: length of signature
*              - const uint8_t *m: pointer to message
*              - size_t mlen: length of message
*              - const uint8_t *pk: pointer to bit-packed public key
*
* Returns 0 if signature could be verified correctly and -1 otherwise
**************************************************/
int crypto_sign_verify(const uint8_t *sig,
                       size_t siglen,
                       const uint8_t *m,
                       size_t mlen,
                       const uint8_t *pk)
{
#ifdef CONSTRAINED_DEVICE
  unsigned int i;
  uint8_t buf[K*POLYW1_PACKEDBYTES];
  uint8_t mu[CRHBYTES];
  uint8_t c2[SEEDBYTES];
  uint8_t c[SEEDBYTES];
  polyveck w1;
  uint8_t h[N_*K] = {0};
  keccak_state state;

  if(siglen != CRYPTO_BYTES)
    return 1;

  //if signature OK - the h part will be unpacked - else - error
  int err_code = verify_sig_z_h_malform(h, sig);
  if(err_code)
	  return err_code;

  unpack_sig_c(c, sig);
#else
  unsigned int i;
  uint8_t buf[K*POLYW1_PACKEDBYTES];
  uint8_t rho[SEEDBYTES];
  uint8_t mu[CRHBYTES];
  uint8_t c[SEEDBYTES];
  uint8_t c2[SEEDBYTES];
  poly cp;
  polyvecl mat[K], z;
  polyveck t1, w1, h;
  keccak_state state;

  if(siglen != CRYPTO_BYTES)
    return -1;

  unpack_pk(rho, &t1, pk);
  if(unpack_sig(c, &z, &h, sig))
    return -1;
  if(polyvecl_chknorm(&z, GAMMA1 - BETA))
    return -1;
#endif


  /* Compute CRH(CRH(rho, t1), msg) */
  crh(mu, pk, CRYPTO_PUBLICKEYBYTES);
  shake256_init(&state);
  shake256_absorb(&state, mu, CRHBYTES);
  shake256_absorb(&state, m, mlen);
  shake256_finalize(&state);
  shake256_squeeze(mu, CRHBYTES, &state);

#ifdef CONSTRAINED_DEVICE
  /* Matrix-vector multiplication; compute Az - c2^dt1 */
  polyvec_reconstruct_w1_montgomery(&w1, sig, pk);

  /* Reconstruct w1 */
  polyveck_caddq(&w1);
  polyveck_use_hint_r(&w1, &w1, h);
  polyveck_pack_w1(buf, &w1);
#else
  /* Matrix-vector multiplication; compute Az - c2^dt1 */
  poly_challenge(&cp, c);
  polyvec_matrix_expand(mat, rho);

  polyvecl_ntt(&z);
  polyvec_matrix_pointwise_montgomery(&w1, mat, &z);

  poly_ntt(&cp);
  polyveck_shiftl(&t1);
  polyveck_ntt(&t1);
  polyveck_pointwise_poly_montgomery(&t1, &cp, &t1);

  polyveck_sub(&w1, &w1, &t1);
  polyveck_reduce(&w1);
  polyveck_invntt_tomont(&w1);

  /* Reconstruct w1 */
  polyveck_caddq(&w1);
  polyveck_use_hint(&w1, &w1, &h);
  polyveck_pack_w1(buf, &w1);

#endif
  /* Call random oracle and verify challenge */
  shake256_init(&state);
  shake256_absorb(&state, mu, CRHBYTES);
  shake256_absorb(&state, buf, K*POLYW1_PACKEDBYTES);
  shake256_finalize(&state);
  shake256_squeeze(c2, SEEDBYTES, &state);
  for(i = 0; i < SEEDBYTES; ++i)
    if(c[i] != c2[i])
      return 6;

  return 0;
}

/*************************************************
* Name:        crypto_sign_open
*
* Description: Verify signed message.
*
* Arguments:   - uint8_t *m: pointer to output message (allocated
*                            array with smlen bytes), can be equal to sm
*              - size_t *mlen: pointer to output length of message
*              - const uint8_t *sm: pointer to signed message
*              - size_t smlen: length of signed message
*              - const uint8_t *pk: pointer to bit-packed public key
*
* Returns 0 if signed message could be verified correctly and -1 otherwise
**************************************************/
int crypto_sign_open(uint8_t *m,
                     size_t *mlen,
                     const uint8_t *sm,
                     size_t smlen,
                     const uint8_t *pk)
{
  size_t i;
  int err_code = -1;

  if(smlen < CRYPTO_BYTES)
    goto badsig;

  *mlen = smlen - CRYPTO_BYTES;
  err_code = crypto_sign_verify(sm, CRYPTO_BYTES, sm + CRYPTO_BYTES, *mlen, pk);
  if(err_code)
    goto badsig;
  else {
    /* All good, copy msg, return 0 */
    for(i = 0; i < *mlen; ++i)
      m[i] = sm[CRYPTO_BYTES + i];
    return 0;
  }

badsig:
  /* Signature verification failed */
  *mlen = -1;
  for(i = 0; i < smlen; ++i)
    m[i] = 0;

  return err_code;
}