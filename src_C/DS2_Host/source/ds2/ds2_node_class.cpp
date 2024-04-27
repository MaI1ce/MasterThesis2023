#include "ds2_node_class.h"

#include <cstdio>

#include <string.h>
#include <intrin.h>

extern "C" {
#include "./inc/commit.h"
#include "./inc/fips202.h"
#include "./inc/rand.h"
#include "./inc/reduce.h"
}

static uint8_t xorSign(const char* pmessage, uint32_t message_len)
{
	uint8_t seed = 0x00;
	for (uint32_t i = 0x00; i < message_len; i++)
		seed = (uint8_t)pmessage[i] ^ seed;
	return seed;
}

static void poly_gen_commit2(poly_t ck[][TC_COLS], poly_t r[][TC_COLS], poly_t fi[][_K_])
{
	memset(fi, 0, _K_ * _K_ * _N_);

	for (size_t k = 0; k < TC_COLS; k++) {
		for (size_t j = 0; j < _K_; j++) {
			for (size_t i = 0; i < _K_; i++) {
				// f[i][j] += r[j][k] * ck[i][k]
				for (size_t n = 0; n < _N_; n++)
					fi[i][j].coeffs[n] += montgomery_reduce((int64_t)ck[i][k].coeffs[n] * r[j][k].coeffs[n]);
			}
		}
	}
	poly_reduce((poly_t*)fi, _K_ * _K_);
	poly_invntt_tomont((poly_t*)fi, _K_ * _K_);
}



std::string ds2_node::get_pi_commit()
{
	uint32_t* seed_ptr = (uint32_t*)self.pi_val;

	//generate pi
	for (int i = 0; i < (SEED_BYTES / sizeof(uint32_t)); i++)
	{
		*seed_ptr = rand();
		seed_ptr++;
	}
	// generate pi commit
	h1(self.pi_val, node_id, self.pi_commit);

	return std::string((const char*)self.pi_commit, DS2_Pi_COMMIT_SIZE);
}


std::string ds2_node::get_pi_val()
{
	return std::string((const char*)self.pi_val, DS2_Pi_VALUE_SIZE);
}

std::string ds2_node::get_ti_commit()
{
	poly_t s1_[_L_];
	//generate A
	poly_uniform(rho, _K_ * _L_, 0, &A[0][0]);

	//generate private seed
	uint32_t* seed_ptr = (uint32_t*)private_seed;
	for (int i = 0; i < (SEED_BYTES / sizeof(uint32_t)); i++)
	{
		*seed_ptr = rand();
		seed_ptr++;
	}

	//generate private key
	poly_eta(private_seed, 0, _L_, s1);
	poly_eta(private_seed, _L_, _K_, s2);

	// Compute ti = (A | I) * s_1 + s_2
	poly_copy(s1, _L_, s1_);

	poly_ntt(s1_, _L_);
	poly_product(A, s1_, t1);
	poly_reduce(t1, _K_);
	poly_invntt_tomont(t1, _K_);

	poly_add(t1, s2, _K_, t1);
	poly_addq(t1, _K_);
	poly_power2round(t1, _K_, t0);

	poly_pack(T1_BITS, t1, _K_, self.ti_val);

	//generate ti commit
	h2(self.ti_val, node_id, self.ti_commit);

	return std::string((const char*)self.ti_commit, DS2_Ti_COMMIT_SIZE);
}

std::string ds2_node::get_ti_val()
{
	return std::string((const char*)self.ti_val, DS2_Ti_VALUE_SIZE);
}

void ds2_node::set_rho(const std::string& rho_)
{
	memcpy(rho, rho_.c_str(), SEED_BYTES);
}

void ds2_node::set_tr(const std::string& tr_)
{
	memcpy(tr, tr_.c_str(), SEED_BYTES);
}

void ds2_node::set_msg(const std::string& msg_)
{
	memcpy(msg_buff, msg_.c_str(), MSG_SIZE);
}

void ds2_node::set_c(const std::string& c_)
{
	memcpy(c, c_.c_str(), SEED_BYTES);
}

std::string ds2_node::get_fi_commit()
{
	static uint32_t nonce_y = 0;
	uint32_t* y_seed_ptr;
	uint32_t* r_seed_ptr;

	poly_t y1_[_L_] = { 0 };

	y_seed_ptr = (uint32_t*)y_seed;
	r_seed_ptr = (uint32_t*)r_seed;
	for (int i = 0; i < (SEED_BYTES / sizeof(uint32_t)); i++)
	{
		*y_seed_ptr = rand();
		y_seed_ptr++;

		*r_seed_ptr = rand();
		r_seed_ptr++;
	}

	keccak_state_t state;
	keccak_init(&state);
	shake256_absorb(&state, msg_buff, MSG_SIZE);
	shake256_absorb(&state, tr, SEED_BYTES);
	shake256_finalize(&state);
	shake256_squeeze(&state, SEED_BYTES, ck_seed);

	//generate y1 and y2
	poly_normal(y_seed, nonce_y, SIGMA, _L_, y1);
	nonce_y += _L_;
	poly_normal(y_seed, nonce_y, SIGMA, _K_, y2);
	nonce_y += _K_;

	// Compute w_n = (A | I) * y_n
	poly_copy(y1, _L_, y1_);
	poly_ntt(y1_, _L_);

	poly_product(A, y1_, w);
	poly_reduce(w, _K_);
	poly_invntt_tomont(w, _K_);

	poly_add(w, y2, _K_, w);
	poly_freeze(w, _K_);

	for (size_t k = 0; k < TC_COLS; k++) { //WARNING !!!
		for (size_t i = 0; i < _K_; i++) {
			poly_uniform(ck_seed, 1, i * TC_COLS + k, &this->ck[i][k]);
		}
	}

	for (size_t k = 0; k < TC_COLS; k++) { //WARNING !!!
		for (size_t j = 0; j < _K_; j++) {
			uint32_t nonce = 0;
			do {
				nonce++;
				sample_normal_from_seed(r_seed, j * TC_COLS + k + nonce, 0, TC_S, _N_, ri[j][k].coeffs);
			} while (!poly_check_norm(&ri[j][k], 1, TC_B));
		}
	}

	std::printf("A = %d\n", xorSign((const char*)A, sizeof(A)));
	std::printf("ck_seed = %d\n", xorSign((const char*)ck_seed, sizeof(ck_seed)));
	std::printf("ck = %d\n", xorSign((const char*)ck, sizeof(ck)));
	std::printf("r_seed = %d\n", xorSign((const char*)r_seed, sizeof(r_seed)));
	std::printf("r[%d] = %d\n", node_id, xorSign((const char*)ri, sizeof(ri)));
	std::printf("t1[%d] = %d\n", node_id, xorSign((const char*)t1, sizeof(t1)));
	std::printf("w[%d] = %d\n", node_id, xorSign((const char*)w, sizeof(w)));

	std::printf("ck_seed = %d %d %d %d\n", *(uint32_t*)&ck_seed[0], 
		*(uint32_t*)&ck_seed[4],
		*(uint32_t*)&ck_seed[8],
		*(uint32_t*)&ck_seed[12]);

	std::printf("r_seed = %d %d %d %d\n", *(uint32_t*)&r_seed[0],
		*(uint32_t*)&r_seed[4],
		*(uint32_t*)&r_seed[8],
		*(uint32_t*)&r_seed[12]);

	poly_gen_commit(ck_seed, r_seed, Fi);
	std::printf("f[%d] = %d\n", node_id, xorSign((const char*)Fi, sizeof(Fi)));

	poly_add((poly_t*) & Fi[1], w, _K_, (poly_t*)&Fi[1]);
	poly_freeze((poly_t*)Fi, _K_ * _K_);

	poly_pack(TC_L, (poly_t*)Fi, _K_ * _K_, self.fi_commit);


	return std::string((const char*)self.fi_commit, DS2_Fi_COMMIT_SIZE);
}

std::string ds2_node::get_zi_1_val()
{
	poly_t cs1[_L_] = { 0 };
	poly_t cs2[_K_] = { 0 };

	poly_t t0_[_K_] = { 0 };

	memset(&poly_c, 0, sizeof(poly_c));
	memset(z1, 0, sizeof(z1));
	memset(z2, 0, sizeof(z2));

	poly_challenge(c, &poly_c);

	// z_n = c * s_n + y_n
	poly_copy(s1, _L_, cs1);
	poly_copy(s2, _K_, cs2);

	poly_ntt(cs1, _L_);
	poly_ntt(cs2, _K_);
	poly_ntt(&poly_c, 1);

	poly_mul_pointwise(cs1, &poly_c, _L_, cs1);
	poly_mul_pointwise(cs2, &poly_c, _K_, cs2);

	poly_reduce(cs1, _L_);
	poly_reduce(cs2, _K_);

	poly_invntt_tomont(cs1, _L_);
	poly_invntt_tomont(cs2, _K_);

	poly_add(cs1, y1, _L_, z1);
	poly_add(cs2, y2, _K_, z2);

	uint8_t rej = 0;
	rej = poly_reject(z1, z2, cs1, cs2);
	//std::printf("reject 1 = %d\n", rej);
	//APP_DBG("RFD DS2 -- SIGN -- REJECT RESULT 1 %d", rej);

	//APP_DBG("RFD DS2 -- SIGN -- z2 = %ld", z2[1].coeffs[_N - 1]);

	poly_copy(t0, _K_, t0_);
	poly_ntt(t0_, _K_);
	//APP_DBG("RFD DS2 -- SIGN -- t0 = %ld", t0_[1].coeffs[_N - 1]);
	poly_mul_pointwise(t0_, &poly_c, _K_, t0_);
	poly_reduce(t0_, _K_);
	poly_invntt_tomont(t0_, _K_);

	//APP_DBG("RFD DS2 -- SIGN -- t0 = %ld", t0_[1].coeffs[_N - 1]);
	//APP_DBG("RFD DS2 -- SIGN -- z2 = %ld", z2[1].coeffs[_N - 1]);


	poly_sub(z2, t0_, _K_, z2);

	poly_center(z1, _L_);
	poly_center(z2, _K_);

	//APP_DBG("RFD DS2 -- SIGN -- t0 = %ld", t0_[1].coeffs[_N - 1]);
	//APP_DBG("RFD DS2 -- SIGN -- z2 = %ld", z2[1].coeffs[_N - 1]);
	std::printf("z1[%d] = %d\n", node_id, xorSign((const char*)z1, sizeof(z1)));
	std::printf("z2[%d] = %d\n", node_id, xorSign((const char*)z2, sizeof(z2)));
	std::printf("poly_c = %d\n", xorSign((const char*)&poly_c, sizeof(poly_c)));

	//check norm
	rej |= !poly_check_norm(z1, _L_, _B_);
	//std::printf("reject 2 = %d\n", rej);
	//APP_DBG("RFD DS2 -- SIGN -- REJECT RESULT 2 %d", rej);

	rej |= !poly_check_norm(z2, _K_, _B_);
	//std::printf("reject 3 = %d\n", rej);

	//APP_DBG("RFD DS2 -- SIGN -- REJECT RESULT 3 %d", rej);

	poly_addq(z1, _L_);
	poly_pack(TC_L, z1, _L_, self.zi_1_val);

	poly_addq(z2, _K_);
	poly_pack(TC_L, z2, _L_, self.zi_2_val);

	return std::string((const char*)self.zi_1_val, DS2_Zi_1_VALUE_SIZE);
}

std::string ds2_node::get_zi_2_val()
{
	return std::string((const char*)self.zi_2_val, DS2_Zi_2_VALUE_SIZE);
}


std::string ds2_node::get_ri_val()
{
	return std::string((const char*)r_seed, SEED_BYTES);
}

void ds2_node::reset()
{
	memset(rho, 0, SEED_BYTES);
	memset(A, 0 , sizeof(A));
	memset(tr, 0, sizeof(tr));
	memset(c, 0, sizeof(c));
	memset(&poly_c, 0, sizeof(poly_c));
	memset(s1, 0, sizeof(s1));
	memset(s2, 0, sizeof(s2));
	memset(y1, 0, sizeof(y1));
	memset(y2, 0, sizeof(y2));
	memset(z1, 0, sizeof(z1));
	memset(z2, 0, sizeof(z2));
	memset(t0, 0, sizeof(t0));
	memset(t1, 0, sizeof(t1));
	memset(Fi, 0, sizeof(Fi));
	memset(r_seed, 0, sizeof(r_seed));
	memset(ck_seed, 0, sizeof(ck_seed));
	memset(ri, 0, sizeof(ri));
	memset(ck, 0, sizeof(ck));
}