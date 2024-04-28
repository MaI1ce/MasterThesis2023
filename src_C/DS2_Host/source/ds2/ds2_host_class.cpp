#include "ds2_host_class.h"

#include <cstdio>

extern "C" {
#include "./inc/commit.h"
#include "./inc/fips202.h"
#include "./inc/rand.h"
#include "./inc/reduce.h"
}

#include <string.h>
#include <intrin.h>

static uint8_t xorSign(const char* pmessage, uint32_t message_len)
{
	uint8_t seed = 0x00;
	for (uint32_t i = 0x00; i < message_len; i++)
		seed = (uint8_t)pmessage[i] ^ seed;
	return seed;
}


// Function to get the CPU cycle count
static inline uint64_t get_timestamp() {

#ifdef __WINDOWS__
/*
	Generates the rdtsc instruction, which returns the processor time stamp.
	The processor time stamp records the number of clock cycles since the last reset.

	The __rdtsc() instruction returns the number of ticks from the timestamp counter (TSC), 
	which corresponds to the number of CPU cycles. The TSC is a high-resolution counter that increments 
	with each clock cycle of the CPU. It does not rely on any external frequency and provides 
	a direct measurement of elapsed time in terms of cycles.
*/
	return __rdtsc();

	//LARGE_INTEGER timestamp;
	//QueryPerformanceCounter(&timestamp);
	//return timestamp.QuadPart;

	//if(QueryPerformanceFrequency(&frequency) != FALSE)
	//	double interval = static_cast<double>(end.QuadPart - start.QuadPart) / frequency.QuadPart;

#elif defined(__LINUX__)
	return 0;
#endif


}

uint64_t ds2_host::get_timer()
{
	return get_timestamp();
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

bool ds2_host::check_commit(const std::string& r, const std::string& ck, const std::string& fi, const std::string& wi, uint64_t& timestamp)
{
	poly_t f1[_K_][_K_] = { 0 };
	poly_t f2[_K_][_K_] = { 0 };
	poly_t w[_K_] = { 0 };

	uint8_t ri[SEED_BYTES] = { 0 };
	uint8_t cki[SEED_BYTES] = { 0 };

	timestamp = get_timestamp();//start

	memcpy(f2, fi.c_str(), fi.size());
	memcpy(w, wi.c_str(), wi.size());
	memcpy(ri, r.c_str(), r.size());
	memcpy(cki, ck.c_str(), ck.size());

	poly_gen_commit(cki, ri, f1);

	poly_add((poly_t*)&f1[1], (poly_t*)w, _K_, (poly_t*)&f1[1]);

	poly_freeze((poly_t*)f1, _K_ * _K_);

	//timestamp = static_cast<uint64_t>(std::round(coef * (get_timestamp() - timestamp)));//finish
	timestamp = get_timestamp() - timestamp;
	if (memcmp(f1, f2, sizeof(f1)) == 0)
		return true;
	else
		return false;
}

bool ds2_host::check_commit2(const std::string& r, const std::string& ck, const std::string& fi, const std::string& wi, uint64_t& timestamp)
{
	poly_t f1[_K_][_K_] = { 0 };
	poly_t f2[_K_][_K_] = { 0 };
	poly_t w[_K_] = { 0 };

	uint8_t ri[SEED_BYTES] = { 0 };
	uint8_t cki[SEED_BYTES] = { 0 };

	timestamp = get_timestamp();//start

	memcpy(f2, fi.c_str(), fi.size());
	memcpy(w, wi.c_str(), wi.size());
	memcpy(ri, r.c_str(), r.size());
	memcpy(ck_seed, ck.c_str(), ck.size());

	for (size_t k = 0; k < TC_COLS; k++) { //WARNING !!!
		for (size_t i = 0; i < _K_; i++) {
			poly_uniform(ck_seed, 1, i * TC_COLS + k, &this->ck[i][k]);
		}
	}

	uint32_t nonce = 0;
	for (size_t k = 0; k < TC_COLS; k++) { //WARNING !!!
		for (size_t j = 0; j < _K_; j++) {
			nonce = 0;
			do {
				nonce++;
				sample_normal_from_seed(ri, j * TC_COLS + k + nonce, 0, TC_S, _N_, this->ri[j][k].coeffs);
			} while (!poly_check_norm(&this->ri[j][k], 1, TC_B));
		}
	}

	poly_gen_commit2(this->ck, this->ri, f1);

	poly_add((poly_t*)&f1[1], (poly_t*)w, _K_, (poly_t*)&f1[1]);

	poly_freeze((poly_t*)f1, _K_ * _K_);

	//timestamp = static_cast<uint64_t>(std::round(coef * (get_timestamp() - timestamp)));//finish

	timestamp = get_timestamp() - timestamp;

	if (memcmp(f1, f2, sizeof(f1)) == 0)
		return true;
	else
		return false;
}

std::string ds2_host::hash_msg(const std::string& msg)
{
#define HASH_SIZE 256
	char msg_hash[HASH_SIZE] = { 0 };
	keccak_state_t state;
	keccak_init(&state);
	shake256_absorb(&state, (const uint8_t*)msg.c_str(), msg.size());
	shake256_finalize(&state);
	shake256_squeeze(&state, HASH_SIZE, (uint8_t*)msg_hash);

	return std::string(msg_hash, HASH_SIZE);
}

std::string ds2_host::get_rho(uint64_t& timestamp)
{
	timestamp = get_timestamp();//start

	//rho = H(pi)
	keccak_state_t state;
	keccak_init(&state);
	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++)
	{
		shake128_absorb(&state, parties[i].pi_val, SEED_BYTES);
	}
	shake128_finalize(&state);
	shake128_squeeze(&state, SEED_BYTES, rho);

	poly_uniform(rho, _K_ * _L_, 0, &A[0][0]);

	//timestamp = static_cast<uint64_t>(std::round(coef * (get_timestamp() - timestamp)));//finish
	timestamp = get_timestamp() - timestamp;

	return std::string((const char*)rho, sizeof(rho));
}

std::string ds2_host::get_tr(uint64_t& timestamp) 
{

	poly_t temp_ti[_K_] = { 0 };

	timestamp = get_timestamp();//start

	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++) {
		poly_unpack(T1_BITS, parties[i].ti_val, _K_, 0, temp_ti);
		poly_add(t1, temp_ti, _K_, t1);
	}
	poly_freeze(t1, _K_);

	//generate tr
	keccak_state_t state;
	keccak_init(&state);
	shake128_absorb(&state, rho, SEED_BYTES);
	shake128_absorb(&state, (uint8_t*)t1, sizeof(t1));
	shake128_finalize(&state);
	shake128_squeeze(&state, SEED_BYTES, tr);

	//poly_pack(T1_BITS, t1, _K_, t1_packed);

	//timestamp = static_cast<uint64_t>(std::round(coef * (get_timestamp() - timestamp)));//finish
	timestamp = get_timestamp() - timestamp;
	return std::string((const char*)tr, sizeof(tr));
}

std::string ds2_host::get_c(uint64_t& timestamp)
{
	poly_t Commit[_K_][_K_] = { 0 };
	poly_t fi[_K_][_K_] = { 0 };
	uint8_t f_packed[DS2_Fi_COMMIT_SIZE] = { 0 };

	timestamp = get_timestamp();//start

	//commit = sum(f_i)
	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++) {
		poly_unpack(TC_L, parties[i].fi_commit, _K_ * _K_, 0, (poly_t*)fi);
		poly_add((poly_t*)Commit, (poly_t*)fi, _K_ * _K_, (poly_t*)Commit);
	}
	poly_freeze((poly_t*)Commit, _K_ * _K_);

	poly_pack(TC_L, (poly_t*)Commit, _K_ * _K_, f_packed);

	//c = H(com, msg, pk)
	keccak_state_t state;
	keccak_init(&state);
	shake256_absorb(&state, (const uint8_t*)msg.c_str(), msg.size());
	shake256_absorb(&state, tr, SEED_BYTES);
	shake256_finalize(&state);
	shake256_squeeze(&state, SEED_BYTES, ck_seed);

	keccak_init(&state);
	shake256_absorb(&state, f_packed, sizeof(f_packed));
	shake256_absorb(&state, ck_seed, sizeof(ck_seed));
	shake256_finalize(&state);
	shake256_squeeze(&state, SEED_BYTES, c);

	poly_challenge(c, &poly_c);
	poly_ntt(&poly_c, 1);
	/*
	for (size_t k = 0; k < TC_COLS; k++) { //WARNING !!!
		for (size_t i = 0; i < _K_; i++) {
			poly_uniform(ck_seed, 1, i * TC_COLS + k, &ck[i][k]);
		}
	}*/

	//timestamp = static_cast<uint64_t>(std::round(coef * (get_timestamp() - timestamp)));//finish
	timestamp = get_timestamp() - timestamp;
	return std::string((const char*)c, sizeof(c));
}

std::string ds2_host::get_publick_key()
{
	std::string key_str = std::string((const char*)t1, sizeof(t1));

	key_str += std::string((const char*)rho, sizeof(rho));

	key_str += std::string((const char*)tr, sizeof(tr));

	return key_str;
}

void ds2_host::set_public_key(const std::string& t1_, const std::string& rho_, const std::string& tr_)
{
	memcpy(t1, t1_.c_str(), t1_.size());
	memcpy(tr, tr_.c_str(), tr_.size());
	memcpy(rho, rho_.c_str(), rho_.size());

	poly_uniform(rho, _K_ * _L_, 0, &A[0][0]);
}

std::string ds2_host::get_signature(uint64_t& timestamp)
{
	poly_t z1_temp[_L_] = { 0 };
	poly_t z2_temp[_K_] = { 0 };
	poly_t t1_temp[_K_] = { 0 };
	poly_t w_temp[_K_] = { 0 };
	poly_t F1[_K_][_K_] = { 0 };
	poly_t F2[_K_][_K_] = { 0 };

	timestamp = get_timestamp();

	uint8_t rej = 0;
	/*
	std::printf("A = %d\n", xorSign((const char*)A, sizeof(A)));
	std::printf("ck = %d\n", xorSign((const char*)ck, sizeof(ck)));
	std::printf("ck_seed = %d\n", xorSign((const char*)ck_seed, sizeof(ck_seed)));
	std::printf("poly_c = %d\n", xorSign((const char*)&poly_c, sizeof(poly_c)));
	*/
	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++) {
		poly_unpack(TC_L, parties[i].zi_1_val, _L_, 0, z1_temp);
		poly_unpack(TC_L, parties[i].zi_2_val, _K_, 0, z2_temp);
		/*
		for (size_t k = 0; k < TC_COLS; k++) { //WARNING !!!
			for (size_t j = 0; j < _K_; j++) {
				uint32_t nonce = 0;
				do {
					nonce++;
					sample_normal_from_seed(parties[i].ri_val, j * TC_COLS + k + nonce, 0, TC_S, _N_, ri[j][k].coeffs);
				} while (!poly_check_norm(&ri[j][k], 1, TC_B));
			}
		}*/
		//std::printf("r_seed = %d\n", xorSign((const char*)parties[i].ri_val, sizeof(parties[i].ri_val)));
		//std::printf("r[%d] = %d\n", i, xorSign((const char*)ri, sizeof(ri)));

		poly_center(z1_temp, _L_);
		poly_center(z2_temp, _K_);

		//std::printf("z1[%d] = %d\n", i, xorSign((const char*)z1_temp, sizeof(z1_temp)));
		//std::printf("z2[%d] = %d\n", i, xorSign((const char*)z2_temp, sizeof(z2_temp)));

		poly_add(z1, z1_temp, _L_, z1);
		poly_add(z2, z2_temp, _K_, z2);

		// w = Az1 - ct1 * 2^D
		poly_unpack(T1_BITS, parties[i].ti_val, _K_, 0, t1_temp);
		//std::printf("t1[%d] = %d\n", i, xorSign((const char*)t1_temp, sizeof(t1_temp)));
		

		// t1 * 2^D
		poly_const_mul(t1_temp, 1 << _D_, _K_, t1_temp);

		poly_ntt(z1_temp, _L_);
		poly_ntt(t1_temp, _K_);

		poly_product(A, z1_temp, w_temp);
		poly_mul_pointwise(t1_temp, &poly_c, _K_, t1_temp);

		poly_reduce(w_temp, _K_);
		poly_reduce(t1_temp, _K_);

		poly_invntt_tomont(w_temp, _K_);
		poly_invntt_tomont(t1_temp, _K_);

		poly_add(w_temp, z2_temp, _K_, w_temp);
		poly_sub(w_temp, t1_temp, _K_, w_temp);

		poly_freeze(w_temp, _K_);

		//std::printf("w[%d] = %d\n", i, xorSign((const char*)w_temp, sizeof(w_temp)));
		// com_i = ck*r + w
		poly_unpack(TC_L, parties[i].fi_commit, _K_ * _K_, 0, (poly_t*)F2);
		/*
		std::printf("ck_seed = %d %d %d %d\n", *(uint32_t*)&ck_seed[0],
			*(uint32_t*)&ck_seed[4],
			*(uint32_t*)&ck_seed[8],
			*(uint32_t*)&ck_seed[12]);
		
		std::printf("r_seed = %d %d %d %d\n", *(uint32_t*)&parties[i].ri_val[0],
			*(uint32_t*)&parties[i].ri_val[4],
			*(uint32_t*)&parties[i].ri_val[8],
			*(uint32_t*)&parties[i].ri_val[12]);*/
		poly_gen_commit(ck_seed, parties[i].ri_val, F1);//WARNING !!!
		
		//poly_gen_commit2(ck, ri, F1);
		//std::printf("f[%d] = %d\n", i, xorSign((const char*)F1, sizeof(F1)));



		poly_add((poly_t*)&F1[1], (poly_t*)w_temp, _K_, (poly_t*)&F1[1]);
		poly_freeze((poly_t*)F1, _K_ * _K_);


		//check commitments
		rej = memcmp(F1, F2, sizeof(F1));

		if (rej != 0) {
			//timestamp = static_cast<uint64_t>(std::round(coef * (get_timestamp() - timestamp)));//finish
			timestamp = get_timestamp() - timestamp;
			char errormsg[80] = { 0 };
			std::sprintf(errormsg, "Commitment check failed for node ID %d", i);
			err_code = DS2_ERROR_Fi_COMMIT;
			throw DS2Exception(DS2_ERROR_Fi_COMMIT, errormsg, timestamp);
		}
	}

	std::string sign_str = std::string((const char*)c, sizeof(c));

	sign_str += std::string((const char*)z1, sizeof(z1));

	sign_str += std::string((const char*)z2, sizeof(z2));

	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++)
	{
		sign_str += std::string((const char*)r[i], sizeof(r[i]));
	}

	//timestamp = static_cast<uint64_t>(std::round(coef * (get_timestamp() - timestamp)));//finish
	timestamp = get_timestamp() - timestamp;

	return sign_str;
}

bool ds2_host::verify(const std::string& c_, const std::string& z1_, const std::string& z2_, const std::vector<std::string> ri_, uint64_t& timestamp)
{
	poly_t F1[_K_][_K_] = { 0 };
	poly_t w_temp[_K_] = { 0 };
	poly_t z1_temp[_K_] = { 0 };
	poly_t t1_temp[_K_] = { 0 };
	uint8_t f_packed[DS2_Fi_COMMIT_SIZE] = { 0 };
	uint8_t new_c[SEED_BYTES] = { 0 };

	timestamp = get_timestamp();

	memset(&poly_c, 0, sizeof(poly_c));
	memcpy(c, c_.c_str(), c_.size()); // ???
	poly_challenge(c, &poly_c);
	poly_ntt(&poly_c, 1);

	//ck = H(com, msg, pk)
	keccak_state_t state;
	keccak_init(&state);
	shake256_absorb(&state, (const uint8_t*)msg.c_str(), msg.size());
	shake256_absorb(&state, tr, SEED_BYTES);
	shake256_finalize(&state);
	shake256_squeeze(&state, SEED_BYTES, ck_seed);

	for (size_t k = 0; k < TC_COLS; k++) { //WARNING !!!
		for (size_t i = 0; i < _K_; i++) {
			poly_uniform(ck_seed, 1, i * TC_COLS + k, &ck[i][k]);
		}
	}

	memcpy(z1, z1_.c_str(), z1_.size());
	memcpy(z1_temp, z1_.c_str(), z1_.size());
	memcpy(z2, z2_.c_str(), z2_.size());

	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++)
	{
		memcpy(r[i], ri_[i].c_str(), DS2_Ri_VALUE_SIZE);
	}

	
	poly_t r_temp = { 0 };
	for (size_t k = 0; k < TC_COLS; k++) { //WARNING !!!
		
		for (size_t j = 0; j < _K_; j++) {
			memset(ri[j][k].coeffs, 0, sizeof(ri[j][k].coeffs));

			for (int i = 0; i < DS2_MAX_PARTY_NUM; i++) {
				uint32_t nonce = 0;
				
				do {
					nonce++;
					sample_normal_from_seed(r[i], j * TC_COLS + k + nonce, 0, TC_S, _N_, r_temp.coeffs);
				} while (!poly_check_norm(&ri[j][k], 1, TC_B));

				poly_add(&(ri[j][k]), &r_temp, 1, &(ri[j][k]));
			}
		}
	}

	// t1 * 2^D
	poly_const_mul(t1, 1 << _D_, _K_, t1_temp);

	poly_ntt(z1_temp, _L_);
	poly_ntt(t1_temp, _K_);

	poly_product(A, z1_temp, w_temp);
	poly_mul_pointwise(t1_temp, &poly_c, _K_, t1_temp);

	poly_reduce(w_temp, _K_);
	poly_reduce(t1_temp, _K_);

	poly_invntt_tomont(w_temp, _K_);
	poly_invntt_tomont(t1_temp, _K_);

	poly_add(w_temp, z2, _K_, w_temp);
	poly_sub(w_temp, t1_temp, _K_, w_temp);

	poly_freeze(w_temp, _K_);


	poly_gen_commit2(ck, ri, F1);//WARNING !!!

	poly_add((poly_t*)&F1[1], (poly_t*)w_temp, _K_, (poly_t*)&F1[1]);

	poly_freeze((poly_t*)F1, _K_ * _K_);

	poly_pack(TC_L, (poly_t*)F1, _K_ * _K_, f_packed);

	keccak_init(&state);
	shake256_absorb(&state, f_packed, sizeof(f_packed));
	shake256_absorb(&state, ck_seed, sizeof(ck_seed));
	shake256_finalize(&state);
	shake256_squeeze(&state, SEED_BYTES, new_c);

	//timestamp = static_cast<uint64_t>(std::round(coef * (get_timestamp() - timestamp)));//finish
	timestamp = get_timestamp() - timestamp;
	if (memcmp(new_c, c, SEED_BYTES) == 0)
		return true;
	else
		return false;
}

void ds2_host::reset()
{
	memset(parties, 0, sizeof(parties));
	memset(t1, 0, sizeof(t1));
	memset(tr, 0, sizeof(tr));
	memset(rho, 0, sizeof(rho));
	memset(A, 0, sizeof(A));
	memset(c, 0, sizeof(c));
	memset(&poly_c, 0, sizeof(poly_c));
	memset(ck_seed, 0, sizeof(ck_seed));
	memset(z1, 0, sizeof(z1));
	memset(z2, 0, sizeof(z2));
}

bool ds2_host::is_flag_ready(uint32_t flag)
{
	uint32_t ready_flag = 0xFFFFFFFF;
	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++) {
		ready_flag &= (parties[i].status & flag);
	}

	if (ready_flag == flag)
		return true;
	else
		return false;
}

void ds2_host::set_pi_commit(uint32_t party_id, const std::string& commit)
{
	if (party_id >= party_num) {
		err_code = DS2_ERROR_INVALID_NODE_ID;
		throw DS2Exception(DS2_ERROR_INVALID_NODE_ID, "party_id exceeds max allowed number of parties!");
	}

	if (commit.size() != DS2_Pi_COMMIT_SIZE) {
		err_code = DS2_ERROR_Pi_COMMIT;
		throw DS2Exception(DS2_ERROR_Pi_COMMIT, "pi_commit has bad buffer size");
	}
	else {
		memcpy(parties[party_id].pi_commit, commit.c_str(), DS2_Pi_COMMIT_SIZE);

		parties[party_id].status |= DS2_Pi_COMMIT_FLAG;
	}
}

void ds2_host::set_pi_val(uint32_t party_id, const std::string& val)
{
	uint8_t temp_commit[DS2_Pi_COMMIT_SIZE] = { 0 };

	if (party_id >= party_num) {
		err_code = DS2_ERROR_INVALID_NODE_ID;
		throw DS2Exception(DS2_ERROR_INVALID_NODE_ID, "party_id exceeds max allowed number of parties!");
	}

	if (val.size() != DS2_Pi_VALUE_SIZE) {
		err_code = DS2_ERROR_Pi_COMMIT;
		throw DS2Exception(DS2_ERROR_Pi_COMMIT, "pi_val has bad buffer size");
	}
	else {
		memcpy(parties[party_id].pi_val, val.c_str(), DS2_Pi_COMMIT_SIZE);
		parties[party_id].status |= DS2_Pi_VALUE_FLAG;
	}

	h1(parties[party_id].pi_val, party_id, temp_commit);
	if (memcmp(temp_commit, parties[party_id].pi_commit, DS2_Pi_COMMIT_SIZE) != 0) {
		err_code = DS2_ERROR_Pi_COMMIT;
		throw DS2Exception(DS2_ERROR_Pi_COMMIT, "pi value do not correspond to provided pi_commit");
	}

}

void ds2_host::set_ti_commit(uint32_t party_id, const std::string& commit)
{
	if (party_id >= party_num) {
		err_code = DS2_ERROR_INVALID_NODE_ID;
		throw DS2Exception(DS2_ERROR_INVALID_NODE_ID, "party_id exceeds max allowed number of parties!");
	}

	if (commit.size() != DS2_Ti_COMMIT_SIZE) {
		err_code = DS2_ERROR_Ti_COMMIT;
		throw DS2Exception(DS2_ERROR_Ti_COMMIT, "ti_commit has bad buffer size");
	}
	else {
		memcpy(parties[party_id].ti_commit, commit.c_str(), DS2_Ti_COMMIT_SIZE);

		parties[party_id].status |= DS2_Ti_COMMIT_FLAG;
	}
}

void ds2_host::set_ti_val(uint32_t party_id, const std::string& val)
{
	uint8_t temp_commit[DS2_Ti_COMMIT_SIZE] = { 0 };

	if (party_id >= party_num) {
		err_code = DS2_ERROR_INVALID_NODE_ID;
		throw DS2Exception(DS2_ERROR_INVALID_NODE_ID, "party_id exceeds max allowed number of parties!");
	}

	if (val.size() != DS2_Ti_VALUE_SIZE) {
		err_code = DS2_ERROR_Ti_COMMIT;
		throw DS2Exception(DS2_ERROR_Ti_COMMIT, "ti_val has bad buffer size");
	}
	else {
		memcpy(parties[party_id].ti_val, val.c_str(), DS2_Ti_VALUE_SIZE);
		parties[party_id].status |= DS2_Ti_VALUE_FLAG;
	}

	h2(parties[party_id].ti_val, party_id, temp_commit);
	if (memcmp(temp_commit, parties[party_id].ti_commit, DS2_Ti_COMMIT_SIZE) != 0) {
		err_code = DS2_ERROR_Ti_COMMIT;
		throw DS2Exception(DS2_ERROR_Ti_COMMIT, "ti value do not correspond to provided ti_commit");
	}

}

void ds2_host::set_fi_commit(uint32_t party_id, const std::string& commit)
{
	if (party_id >= party_num) {
		err_code = DS2_ERROR_INVALID_NODE_ID;
		throw DS2Exception(DS2_ERROR_INVALID_NODE_ID, "party_id exceeds max allowed number of parties!");
	}

	if (commit.size() != DS2_Fi_COMMIT_SIZE) {
		err_code = DS2_ERROR_Fi_COMMIT;
		throw DS2Exception(DS2_ERROR_Fi_COMMIT, "fi_commit has bad buffer size");
	}
	else {
		memcpy(parties[party_id].fi_commit, commit.c_str(), DS2_Fi_COMMIT_SIZE);

		parties[party_id].status |= DS2_Fi_COMMIT_FLAG;
	}
}

void ds2_host::set_zi_1_val(uint32_t party_id, const std::string& val)
{
	if (party_id >= party_num) {
		err_code = DS2_ERROR_INVALID_NODE_ID;
		throw DS2Exception(DS2_ERROR_INVALID_NODE_ID, "party_id exceeds max allowed number of parties!");
	}

	if (val.size() != DS2_Zi_1_VALUE_SIZE) {
		err_code = DS2_ERROR_Zi_REJECT;
		throw DS2Exception(DS2_ERROR_Zi_REJECT, "zi_1_val has bad buffer size");
	}
	else {
		memcpy(parties[party_id].zi_1_val, val.c_str(), DS2_Zi_1_VALUE_SIZE);

		parties[party_id].status |= DS2_Zi_1_VALUE_FLAG;
	}
}


void ds2_host::set_zi_2_val(uint32_t party_id, const std::string& val)
{
	if (party_id >= party_num) {
		err_code = DS2_ERROR_INVALID_NODE_ID;
		throw DS2Exception(DS2_ERROR_INVALID_NODE_ID, "party_id exceeds max allowed number of parties!");
	}

	if (val.size() != DS2_Zi_2_VALUE_SIZE) {
		err_code = DS2_ERROR_Zi_REJECT;
		throw DS2Exception(DS2_ERROR_Zi_REJECT, "zi_2_val has bad buffer size");
	}
	else {
		memcpy(parties[party_id].zi_2_val, val.c_str(), DS2_Zi_2_VALUE_SIZE);

		parties[party_id].status |= DS2_Zi_2_VALUE_FLAG;
	}
}

void ds2_host::set_ri_val(uint32_t party_id, const std::string& val)
{

	if (party_id >= party_num) {
		err_code = DS2_ERROR_INVALID_NODE_ID;
		throw DS2Exception(DS2_ERROR_INVALID_NODE_ID, "party_id exceeds max allowed number of parties!");
	}

	if (val.size() != DS2_Ri_VALUE_SIZE) {
		err_code = DS2_ERROR_Fi_COMMIT;
		throw DS2Exception(DS2_ERROR_Fi_COMMIT, "ri_val has bad buffer size");
	}
	else {
		memcpy(parties[party_id].ri_val, val.c_str(), DS2_Ri_VALUE_SIZE);
		memcpy(r[party_id], val.c_str(), DS2_Ri_VALUE_SIZE);
		parties[party_id].status |= DS2_Ri_VALUE_FLAG;
	}

}