#include <stdint.h>
#include <string.h>
#include "params.h"
#include "polyvec.h"
#include "poly.h"
#include "packing.h"

#ifdef DEBUG_LOG
#include "usb_debug.h"
#endif

/*************************************************
 * ShV
* Name:        matrix_poly_smul_montgomery
*
* Description: Combines implementation of ExpandA and polyvec_matrix_pointwise_montgomery functions.
* 			   Generates matrix A and vector s1 with uniformly random coefficients a_{i,j}
* 			   by performing rejection sampling on the output stream of SHAKE128(rho|j|i) or AES256CTR(rho,j|i).
*			   Generation and computation is performed 'on fly' and 'in place' in order to take less memory
*
* Memory allocation = 4*N*4 = 4Kb
*
* Arguments:   - polyveck t: output vector after multiplication of A*s1+s2
*              - const uint8_t rho[]: byte array containing seed rho for matrix
*              - const uint8_t rhoprime[]: byte array containing seed rhoprime for vectors s1 and s2
**************************************************/
void polyvec_matrix_poly_smul_montgomery(polyveck* t, const uint8_t rho[], const uint8_t rhoprime[]) {
	unsigned int i, j;
	poly acc;
	poly a_ij;
	poly s1_j;
	poly s2_j;

	for (i = 0; i < K; ++i) {
		memset(&acc, 0, sizeof(acc));
		for (j = 0; j < L; ++j) {
			poly_uniform(&a_ij, rho, (i << 8) + j); // generate A[i][j]
			poly_uniform_eta(&s1_j, rhoprime, j);		// generate s1[j]
			poly_ntt(&s1_j);						// transform s1[j] to ntt
			poly_pointwise_montgomery(&a_ij, &a_ij, &s1_j); // multiply A[i][j] on s1[j]
			poly_add(&acc, &acc, &a_ij);
		}
		poly_reduce(&acc);
		poly_invntt_tomont(&acc); // invert accumulated result from ntt
		poly_uniform_eta(&s2_j, rhoprime, L + i); // generate s2[j]
		poly_add(&t->vec[i], &acc, &s2_j);	// add accumulator and s2[j]
	}
}

/*************************************************
 * ShV
* Name:        matrix_poly_ymul_montgomery
*
* Description: Combines implementation of ExpandA and polyvec_matrix_pointwise_montgomery functions.
* 			   Generates matrix A and vector s1 with uniformly random coefficients a_{i,j}
* 			   by performing rejection sampling on the output stream of SHAKE128(rho|j|i) or AES256CTR(rho,j|i).
*			   Generation and computation is performed 'on fly' and 'in place' in order to take less memory
*
* Memory allocation = 2*N*4 = 2Kb
*
* Arguments:   - polyveck w: output vector after multiplication of A*y
*              - const uint8_t rho[]: byte array containing seed rho for matrix
*              - const uint8_t rhoprime[]: byte array containing seed rhoprime for vectors y
**************************************************/
void polyvec_matrix_poly_ymul_montgomery(polyveck* w, const uint8_t rho[], const uint8_t rhoprime[], uint16_t nonce) {
	unsigned int i, j;
	poly a_ij;
	poly y_j;
	for (i = 0; i < K; ++i) {
		memset(&w->vec[i], 0, sizeof(uint32_t) * N_);
		for (j = 0; j < L; ++j) {
			poly_uniform(&a_ij, rho, (i << 8) + j); 		// generate A[i][j]
#ifdef DEBUG_LOG
			USB_DEBUG_MSG("a[%d][%d] = ",i, j);
		    for(int k = 0; k < N_; k++){
		    	USB_DEBUG_MSG("%d", a_ij.coeffs[k]);
		    }
		    USB_DEBUG_MSG("\r\n");
#endif
			poly_uniform_gamma1(&y_j, rhoprime, L * nonce + j);	// generate y[j]
#ifdef DEBUG_LOG
			USB_DEBUG_MSG("y[%d] = ", j);
		    for(int k = 0; k < N_; k++){
		    	USB_DEBUG_MSG("%d", y_j.coeffs[k]);
		    }
		    USB_DEBUG_MSG("\r\n");
#endif
			poly_ntt(&y_j);										// transform y[j] to ntt
			poly_pointwise_montgomery(&a_ij, &a_ij, &y_j); // multiply A[i][j] on y[j]
			poly_add(&w->vec[i], &w->vec[i], &a_ij);
		}
		poly_reduce(&w->vec[i]);
		poly_invntt_tomont(&w->vec[i]); // invert accumulated result from ntt
#ifdef DEBUG_LOG
			USB_DEBUG_MSG("w[%d] = ", i);
		    for(int k = 0; k < N_; k++){
		    	USB_DEBUG_MSG("%d", w->vec[i].coeffs[k]);
		    }
		    USB_DEBUG_MSG("\r\n");
#endif
	}
}


/*************************************************
 * ShV
* Name:        polyvec_compute_z_montgomery
*
* Description: Generates vectors y and s1 with uniformly random coefficients and based on challenge polynomial cp computes
* 			   vector of polynomials z = y + cp*s1.
*
* 			   !!! CP polynomial MUST be already in ntt form !!!
*
*			   Generation and computation is performed 'on fly' and 'in place' in order to take less memory
*
* Memory allocation = 2*N*4 = 2Kb
*
* Arguments:   - polyvecl z: output vector
* 			   - const poly *cp: challenge polynomial in ntt form
*              - const uint8_t s_rhoprime[]: byte array containing seed for vector s1
*              - const uint8_t y_rhoprime[]: byte array containing seed for vector y
**************************************************/
void polyvec_compute_z_montgomery(polyvecl* z, const poly* cp, const uint8_t s_rhoprime[], const uint8_t y_rhoprime[], uint16_t nonce) {
	unsigned int j;
	poly s1_j;
	poly y_j;

	for (j = 0; j < L; ++j) {
		poly_uniform_eta(&s1_j, s_rhoprime, j);		// generate s1[j]
#ifdef DEBUG_LOG
		USB_DEBUG_MSG("s1[%d] = ", j);
		for(int k = 0; k < N_; k++){
			USB_DEBUG_MSG("%d", s1_j.coeffs[k]);
		}
		USB_DEBUG_MSG("\r\n");
#endif
		poly_ntt(&s1_j);								// transform s1[j] to ntt
		poly_uniform_gamma1(&y_j, y_rhoprime, L * nonce + j);	// generate y[j]; nonce == j
#ifdef DEBUG_LOG
		USB_DEBUG_MSG("y[%d] = ", j);
		for(int k = 0; k < N_; k++){
			USB_DEBUG_MSG("%d", y_j.coeffs[k]);
		}
		USB_DEBUG_MSG("\r\n");
#endif
		poly_pointwise_montgomery(&s1_j, cp, &s1_j); // multiply cp on s1[j]
		poly_invntt_tomont(&s1_j);					// invert multiplication result

		poly_add(&z->vec[j], &y_j, &s1_j);			// z = y + cp*s1.
		poly_reduce(&z->vec[j]);
#ifdef DEBUG_LOG
		USB_DEBUG_MSG("z[%d] = ", j);
		for(int k = 0; k < N_; k++){
			USB_DEBUG_MSG("%d", z->vec[j].coeffs[k]);
		}
		USB_DEBUG_MSG("\r\n");
#endif
	}
}

/*************************************************
 * ShV
* Name:        polyvec_compute_h_montgomery
*
* Description: Generates vector s2 with uniformly random coefficients and based on challenge polynomial cp computes
* 			   part of vector h = cp*s2.
*
* 			   !!! CP polynomial MUST be already in ntt form !!!
*
*			   Generation and computation is performed 'on fly' and 'in place' in order to take less memory
*
* Memory allocation = N*4 = 1Kb
*
* Arguments:   - polyveck h: output vector
* 			   - const poly *cp: challenge polynomial in ntt form
*              - const uint8_t s_rhoprime[]: byte array containing seed for vector s2
**************************************************/
void polyvec_compute_h_montgomery(polyveck* h, const poly* cp, const uint8_t s_rhoprime[]) {
	unsigned int j;
	poly s2_j;

	for (j = 0; j < K; ++j) {
		poly_uniform_eta(&s2_j, s_rhoprime, L + j);		// generate s2[j]
#ifdef DEBUG_LOG
		USB_DEBUG_MSG("s2[%d] = ", j);
		for(int k = 0; k < N_; k++){
			USB_DEBUG_MSG("%d", s2_j.coeffs[k]);
		}
		USB_DEBUG_MSG("\r\n");
#endif
		poly_ntt(&s2_j);								// transform s2[j] to ntt
		poly_pointwise_montgomery(&h->vec[j], cp, &s2_j); // h_j = cp*s2_j
		poly_invntt_tomont(&h->vec[j]);					// invert multiplication result
		//poly_reduce(&h->vec[j]);
#ifdef DEBUG_LOG
		USB_DEBUG_MSG("h = ");
		for(int k = 0; k < N_; k++){
			USB_DEBUG_MSG("%d", h->vec[j].coeffs[k]);
		}
		USB_DEBUG_MSG("\r\n");
#endif
	}
}


/*************************************************
 * ShV
* Name:        polyvec_reconstruct_w1_montgomery
*
* Description: compute w1 = Az - c*2^d*t1
*
* 			   !!! CP polynomial MUST be already in ntt form !!!
*
*			   Generation and computation is performed 'on fly' and 'in place' in order to take less memory
*
* Memory allocation = 4*N*4 = 4Kb
*
* Arguments:   - polyveck *w: output vector
* 			   - const uint8_t sig[CRYPTO_BYTES]: signature
* 			   - const uint8_t pk[CRYPTO_PUBLICKEYBYTES]: public key
*
**************************************************/
void polyvec_reconstruct_w1_montgomery(polyveck* w, const uint8_t sig[CRYPTO_BYTES], const uint8_t pk[CRYPTO_PUBLICKEYBYTES]) {
	unsigned int i, j;
	uint8_t rho[SEEDBYTES];
	uint8_t c[SEEDBYTES];
	poly cp;
	poly a_ij;
	poly z_j;
	poly t1_i;

	unpack_pk_rho(rho, pk);
	unpack_sig_c(c, sig);
	poly_challenge(&cp, c);
	poly_ntt(&cp);

#ifdef DEBUG_LOG
	USB_DEBUG_MSG("cp_v = ");
	for(int k = 0; k < N_; k++){
		USB_DEBUG_MSG("%d", cp.coeffs[k]);
	}
	USB_DEBUG_MSG("\r\n");

    USB_DEBUG_MSG("rho = ");
    for (int k = 0; k < SEEDBYTES; k++) {
        USB_DEBUG_MSG("%d",rho[k]);
    }
    USB_DEBUG_MSG("\r\n");
#endif

	for (i = 0; i < K; ++i) {
		memset(&w->vec[i], 0, sizeof(uint32_t) * N_);
		for (j = 0; j < L; ++j) {
			poly_uniform(&a_ij, rho, (i << 8) + j); 		// generate A[i][j]
			unpack_sig_z(&z_j, j, sig);						// extract z_j from signature
			poly_ntt(&z_j);									// transform z[j] to ntt
#ifdef DEBUG_LOG
			USB_DEBUG_MSG("a[%d][%d] = ", i, j);
			for (int k = 0; k < N_; k++) {
				USB_DEBUG_MSG("%d", a_ij.coeffs[k]);
			}
			USB_DEBUG_MSG("\r\n");
			USB_DEBUG_MSG("z[%d] = ", j);
			for (int k = 0; k < N_; k++) {
				USB_DEBUG_MSG("%d", z_j.coeffs[k]);
			}
			USB_DEBUG_MSG("\r\n");
#endif
			poly_pointwise_montgomery(&a_ij, &a_ij, &z_j);  // multiply A[i][j] on z[j]
			poly_add(&w->vec[i], &w->vec[i], &a_ij);		// accumulate result
		}
		// compute c*2^d*t1
		unpack_pk_t1(&t1_i, i, pk);
#ifdef DEBUG_LOG
		USB_DEBUG_MSG("t1[%d] = ", i);
		for (int k = 0; k < N_; k++) {
			USB_DEBUG_MSG("%d", t1_i.coeffs[k]);
		}
		USB_DEBUG_MSG("\r\n");
#endif
		poly_shiftl(&t1_i);
		poly_ntt(&t1_i);
		poly_pointwise_montgomery(&t1_i, &cp, &t1_i);

		poly_sub(&w->vec[i], &w->vec[i], &t1_i);
		poly_reduce(&w->vec[i]);
		poly_invntt_tomont(&w->vec[i]); // invert accumulated result from ntt
#ifdef DEBUG_LOG
		USB_DEBUG_MSG("w[%d] = ", i);
		for (int k = 0; k < N_; k++) {
			USB_DEBUG_MSG("%d", w->vec[i].coeffs[k]);
		}
		USB_DEBUG_MSG("\r\n");
#endif
	}
}


/*************************************************
* Name:        expand_mat
*
* Description: Implementation of ExpandA. Generates matrix A with uniformly
*              random coefficients a_{i,j} by performing rejection
*              sampling on the output stream of SHAKE128(rho|j|i)
*              or AES256CTR(rho,j|i).
*
* Arguments:   - polyvecl mat[K]: output matrix
*              - const uint8_t rho[]: byte array containing seed rho
**************************************************/
void polyvec_matrix_expand(polyvecl mat[K], const uint8_t rho[SEEDBYTES]) {
  unsigned int i, j;

  for(i = 0; i < K; ++i)
    for(j = 0; j < L; ++j)
      poly_uniform(&mat[i].vec[j], rho, (i << 8) + j);
}

void polyvec_matrix_pointwise_montgomery(polyveck *t, const polyvecl mat[K], const polyvecl *v) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    polyvecl_pointwise_acc_montgomery(&t->vec[i], &mat[i], v);
}

/**************************************************************/
/************ Vectors of polynomials of length L **************/
/**************************************************************/

void polyvecl_uniform_eta(polyvecl *v, const uint8_t seed[SEEDBYTES], uint16_t nonce) {
  unsigned int i;

  for(i = 0; i < L; ++i)
    poly_uniform_eta(&v->vec[i], seed, nonce++);
}

void polyvecl_uniform_gamma1(polyvecl *v, const uint8_t seed[SEEDBYTES], uint16_t nonce) {
  unsigned int i;

  for(i = 0; i < L; ++i)
    poly_uniform_gamma1(&v->vec[i], seed, L*nonce + i);
}

void polyvecl_reduce(polyvecl *v) {
  unsigned int i;

  for(i = 0; i < L; ++i)
    poly_reduce(&v->vec[i]);
}

/*************************************************
* Name:        polyvecl_freeze
*
* Description: Reduce coefficients of polynomials in vector of length L
*              to standard representatives.
*
* Arguments:   - polyvecl *v: pointer to input/output vector
**************************************************/
void polyvecl_freeze(polyvecl *v) {
  unsigned int i;

  for(i = 0; i < L; ++i)
    poly_freeze(&v->vec[i]);
}

/*************************************************
* Name:        polyvecl_add
*
* Description: Add vectors of polynomials of length L.
*              No modular reduction is performed.
*
* Arguments:   - polyvecl *w: pointer to output vector
*              - const polyvecl *u: pointer to first summand
*              - const polyvecl *v: pointer to second summand
**************************************************/
void polyvecl_add(polyvecl *w, const polyvecl *u, const polyvecl *v) {
  unsigned int i;

  for(i = 0; i < L; ++i)
    poly_add(&w->vec[i], &u->vec[i], &v->vec[i]);
}

/*************************************************
* Name:        polyvecl_ntt
*
* Description: Forward NTT of all polynomials in vector of length L. Output
*              coefficients can be up to 16*Q larger than input coefficients.
*
* Arguments:   - polyvecl *v: pointer to input/output vector
**************************************************/
void polyvecl_ntt(polyvecl *v) {
  unsigned int i;

  for(i = 0; i < L; ++i)
    poly_ntt(&v->vec[i]);
}

void polyvecl_invntt_tomont(polyvecl *v) {
  unsigned int i;

  for(i = 0; i < L; ++i)
    poly_invntt_tomont(&v->vec[i]);
}

void polyvecl_pointwise_poly_montgomery(polyvecl *r, const poly *a, const polyvecl *v) {
  unsigned int i;

  for(i = 0; i < L; ++i)
    poly_pointwise_montgomery(&r->vec[i], a, &v->vec[i]);
}

/*************************************************
* Name:        polyvecl_pointwise_acc_montgomery
*
* Description: Pointwise multiply vectors of polynomials of length L, multiply
*              resulting vector by 2^{-32} and add (accumulate) polynomials
*              in it. Input/output vectors are in NTT domain representation.
*
* Arguments:   - poly *w: output polynomial
*              - const polyvecl *u: pointer to first input vector
*              - const polyvecl *v: pointer to second input vector
**************************************************/
void polyvecl_pointwise_acc_montgomery(poly *w,
                                       const polyvecl *u,
                                       const polyvecl *v)
{
  unsigned int i;
  poly t;

  poly_pointwise_montgomery(w, &u->vec[0], &v->vec[0]);
  for(i = 1; i < L; ++i) {
    poly_pointwise_montgomery(&t, &u->vec[i], &v->vec[i]);
    poly_add(w, w, &t);
  }
}

/*************************************************
* Name:        polyvecl_chknorm
*
* Description: Check infinity norm of polynomials in vector of length L.
*              Assumes input polyvecl to be reduced by polyvecl_reduce().
*
* Arguments:   - const polyvecl *v: pointer to vector
*              - int32_t B: norm bound
*
* Returns 0 if norm of all polynomials is strictly smaller than B <= (Q-1)/8
* and 1 otherwise.
**************************************************/
int polyvecl_chknorm(const polyvecl *v, int32_t bound)  {
  unsigned int i;

  for(i = 0; i < L; ++i)
    if(poly_chknorm(&v->vec[i], bound))
      return 1;

  return 0;
}

/**************************************************************/
/************ Vectors of polynomials of length K **************/
/**************************************************************/

void polyveck_uniform_eta(polyveck *v, const uint8_t seed[SEEDBYTES], uint16_t nonce) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_uniform_eta(&v->vec[i], seed, nonce++);
}

/*************************************************
* Name:        polyveck_reduce
*
* Description: Reduce coefficients of polynomials in vector of length K
*              to representatives in [-6283009,6283007].
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_reduce(polyveck *v) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_reduce(&v->vec[i]);
}

/*************************************************
* Name:        polyveck_caddq
*
* Description: For all coefficients of polynomials in vector of length K
*              add Q if coefficient is negative.
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_caddq(polyveck *v) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_caddq(&v->vec[i]);
}

/*************************************************
* Name:        polyveck_freeze
*
* Description: Reduce coefficients of polynomials in vector of length K
*              to standard representatives.
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_freeze(polyveck *v)  {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_freeze(&v->vec[i]);
}

/*************************************************
* Name:        polyveck_add
*
* Description: Add vectors of polynomials of length K.
*              No modular reduction is performed.
*
* Arguments:   - polyveck *w: pointer to output vector
*              - const polyveck *u: pointer to first summand
*              - const polyveck *v: pointer to second summand
**************************************************/
void polyveck_add(polyveck *w, const polyveck *u, const polyveck *v) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_add(&w->vec[i], &u->vec[i], &v->vec[i]);
}

/*************************************************
* Name:        polyveck_sub
*
* Description: Subtract vectors of polynomials of length K.
*              No modular reduction is performed.
*
* Arguments:   - polyveck *w: pointer to output vector
*              - const polyveck *u: pointer to first input vector
*              - const polyveck *v: pointer to second input vector to be
*                                   subtracted from first input vector
**************************************************/
void polyveck_sub(polyveck *w, const polyveck *u, const polyveck *v) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_sub(&w->vec[i], &u->vec[i], &v->vec[i]);
}

/*************************************************
* Name:        polyveck_shiftl
*
* Description: Multiply vector of polynomials of Length K by 2^D without modular
*              reduction. Assumes input coefficients to be less than 2^{31-D}.
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_shiftl(polyveck *v) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_shiftl(&v->vec[i]);
}

/*************************************************
* Name:        polyveck_ntt
*
* Description: Forward NTT of all polynomials in vector of length K. Output
*              coefficients can be up to 16*Q larger than input coefficients.
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_ntt(polyveck *v) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_ntt(&v->vec[i]);
}

/*************************************************
* Name:        polyveck_invntt_tomont
*
* Description: Inverse NTT and multiplication by 2^{32} of polynomials
*              in vector of length K. Input coefficients need to be less
*              than 2*Q.
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_invntt_tomont(polyveck *v) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_invntt_tomont(&v->vec[i]);
}

void polyveck_pointwise_poly_montgomery(polyveck *r, const poly *a, const polyveck *v) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_pointwise_montgomery(&r->vec[i], a, &v->vec[i]);
}


/*************************************************
* Name:        polyveck_chknorm
*
* Description: Check infinity norm of polynomials in vector of length K.
*              Assumes input polyveck to be reduced by polyveck_reduce().
*
* Arguments:   - const polyveck *v: pointer to vector
*              - int32_t B: norm bound
*
* Returns 0 if norm of all polynomials are strictly smaller than B <= (Q-1)/8
* and 1 otherwise.
**************************************************/
int polyveck_chknorm(const polyveck *v, int32_t bound) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    if(poly_chknorm(&v->vec[i], bound))
      return 1;

  return 0;
}

/*************************************************
* Name:        polyveck_power2round
*
* Description: For all coefficients a of polynomials in vector of length K,
*              compute a0, a1 such that a mod^+ Q = a1*2^D + a0
*              with -2^{D-1} < a0 <= 2^{D-1}. Assumes coefficients to be
*              standard representatives.
*
* Arguments:   - polyveck *v1: pointer to output vector of polynomials with
*                              coefficients a1
*              - polyveck *v0: pointer to output vector of polynomials with
*                              coefficients a0
*              - const polyveck *v: pointer to input vector
**************************************************/
void polyveck_power2round(polyveck *v1, polyveck *v0, const polyveck *v) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_power2round(&v1->vec[i], &v0->vec[i], &v->vec[i]);
}

/*************************************************
* Name:        polyveck_decompose
*
* Description: For all coefficients a of polynomials in vector of length K,
*              compute high and low bits a0, a1 such a mod^+ Q = a1*ALPHA + a0
*              with -ALPHA/2 < a0 <= ALPHA/2 except a1 = (Q-1)/ALPHA where we
*              set a1 = 0 and -ALPHA/2 <= a0 = a mod Q - Q < 0.
*              Assumes coefficients to be standard representatives.
*
* Arguments:   - polyveck *v1: pointer to output vector of polynomials with
*                              coefficients a1
*              - polyveck *v0: pointer to output vector of polynomials with
*                              coefficients a0
*              - const polyveck *v: pointer to input vector
**************************************************/
void polyveck_decompose(polyveck *v1, polyveck *v0, const polyveck *v) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_decompose(&v1->vec[i], &v0->vec[i], &v->vec[i]);
}

/*************************************************
* Name:        polyveck_make_hint
*
* Description: Compute hint vector.
*
* Arguments:   - polyveck *h: pointer to output vector
*              - const polyveck *v0: pointer to low part of input vector
*              - const polyveck *v1: pointer to high part of input vector
*
* Returns number of 1 bits.
**************************************************/
unsigned int polyveck_make_hint(polyveck *h,
                                const polyveck *v0,
                                const polyveck *v1)
{
  unsigned int i, s = 0;

  for(i = 0; i < K; ++i)
    s += poly_make_hint(&h->vec[i], &v0->vec[i], &v1->vec[i]);

  return s;
}

/*************************************************
* Name:        polyveck_use_hint
*
* Description: Use hint vector to correct the high bits of input vector.
*
* Arguments:   - polyveck *w: pointer to output vector of polynomials with
*                             corrected high bits
*              - const polyveck *u: pointer to input vector
*              - const polyveck *h: pointer to input hint vector
**************************************************/
void polyveck_use_hint(polyveck *w, const polyveck *u, const polyveck *h) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    poly_use_hint(&w->vec[i], &u->vec[i], &h->vec[i]);
}

void polyveck_use_hint_r(polyveck *w, const polyveck *u, const uint8_t h[N_*K]) {
  unsigned int i;

	for (i = 0; i < K; ++i)
		poly_use_hint_r(&w->vec[i], &u->vec[i], &h[i*N_]);
}

void polyveck_pack_w1(uint8_t r[K*POLYW1_PACKEDBYTES], const polyveck *w1) {
  unsigned int i;

  for(i = 0; i < K; ++i)
    polyw1_pack(&r[i*POLYW1_PACKEDBYTES], &w1->vec[i]);
}
