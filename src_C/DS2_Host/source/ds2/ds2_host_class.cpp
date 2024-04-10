#include "ds2_host_class.h"

#include <cmath>
#include <cstdio>

#ifdef __WINDOWS__
#include <windows.h>
#elif defined(__LINUX__)

#endif

extern "C" {
#include "./inc/commit.h"
#include "./inc/fips202.h"
#include "./inc/rand.h"
#include "./inc/reduce.h"
}

#include <string.h>

// Function to get the CPU cycle count
static inline uint64_t get_cycles() {

#ifdef __WINDOWS__
	LARGE_INTEGER timestamp;
	QueryPerformanceCounter(&timestamp);
	return timestamp.QuadPart;

	//if(QueryPerformanceFrequency(&frequency) != FALSE)
	//	double interval = static_cast<double>(end.QuadPart - start.QuadPart) / frequency.QuadPart;

#elif defined(__LINUX__)
	return 0;
#endif

//	uint64_t t;
//	__asm volatile ("rdtsc" : "=A"(t));
//	return t;
}

void poly_gen_commit2(poly_t ck[][TC_COLS], poly_t r[][TC_COLS], poly_t fi[][_K])
{
	memset(fi, 0, _K * _K * _N_);

	for (size_t k = 0; k < TC_COLS; k++) {
		for (size_t j = 0; j < _K; j++) {
			for (size_t i = 0; i < _K; i++) {
				// f[i][j] += r[j][k] + ck[i][k]
				for (size_t n = 0; n < _N_; n++)
					fi[i][j].coeffs[n] += montgomery_reduce((int64_t)ck[i][k].coeffs[n] * r[j][k].coeffs[n]);
			}
		}
	}
	poly_reduce((poly_t*)fi, _K * _K);
	poly_invntt_tomont((poly_t*)fi, _K * _K);
}

bool ds2_host::check_commit(const std::string& r, const std::string& ck, const std::string& fi, const std::string& wi)
{
	poly_t f1[_K][_K] = { 0 };
	poly_t f2[_K][_K] = { 0 };
	poly_t w[_K] = { 0 };

	memcpy(f1, fi.c_str(), fi.size());

	poly_gen_commit((uint8_t*)ck.c_str(), (uint8_t*)r.c_str(), f2);

	poly_add((poly_t*)&f1[1], (poly_t*)w, _K, (poly_t*)&f1[1]);

	poly_freeze((poly_t*)f1, _K * _K);

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
	timestamp = get_cycles();//start

	//rho = H(pi)
	keccak_state_t state;
	keccak_init(&state);
	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++)
	{
		shake128_absorb(&state, parties[i].pi_val, SEED_BYTES);
	}
	shake128_finalize(&state);
	shake128_squeeze(&state, SEED_BYTES, rho);

	poly_uniform(rho, _K * _L, 0, &A[0][0]);

	timestamp = get_cycles() - timestamp;//finish

	return std::string((const char*)rho, sizeof(rho));
}

std::string ds2_host::get_tr(uint64_t& timestamp) 
{

	poly_t temp_ti[_K] = { 0 };

	timestamp = get_cycles();//start

	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++) {
		poly_unpack(T1_BITS, parties[i].ti_val, _K, 0, temp_ti);
		poly_add(t1, temp_ti, _K, t1);
	}
	poly_freeze(t1, _K);

	//generate tr
	keccak_state_t state;
	keccak_init(&state);
	shake128_absorb(&state, rho, SEED_BYTES);
	shake128_absorb(&state, (uint8_t*)t1, sizeof(t1));
	shake128_finalize(&state);
	shake128_squeeze(&state, SEED_BYTES, tr);

	//poly_pack(T1_BITS, t1, _K, t1_packed);

	timestamp = get_cycles() - timestamp;//finish

	return std::string((const char*)tr, sizeof(tr));
}

std::string ds2_host::get_c(uint64_t& timestamp)
{
	poly_t Commit[_K][_K] = { 0 };
	poly_t fi[_K][_K] = { 0 };
	uint8_t f_packed[DS2_Fi_COMMIT_SIZE] = { 0 };

	timestamp = get_cycles();//start

	//commit = sum(f_i)
	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++) {
		poly_unpack(TC_L, parties[i].fi_commit, _K * _K, 0, (poly_t*)fi);
		poly_add((poly_t*)Commit, (poly_t*)fi, _K * _K, (poly_t*)Commit);
#ifdef DS2_DEBUG
		APP_DBG("FFD DS2 -- SIGN -- com_[%d] = ck*ri + wi = %ld", i, fi[1][1].coeffs[_N_ - 1]);
#endif
	}
	poly_freeze((poly_t*)Commit, _K * _K);

	poly_pack(TC_L, (poly_t*)Commit, _K * _K, f_packed);

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

	for (size_t k = 0; k < TC_COLS; k++) { //WARNING !!!
		for (size_t i = 0; i < _K; i++) {
			poly_uniform(ck_seed, 1, i * TC_COLS + k, &ck[i][k]);
		}
	}

	timestamp = get_cycles() - timestamp;//finish

	return std::string((const char*)c, sizeof(c));
}

std::string ds2_host::get_signature(uint64_t& timestamp)
{
	poly_t z1_temp[_L] = { 0 };
	poly_t z2_temp[_K] = { 0 };
	poly_t t1_temp[_K] = { 0 };
	poly_t w_temp[_K] = { 0 };
	poly_t F1[_K][_K] = { 0 };
	poly_t F2[_K][_K] = { 0 };

	timestamp = get_cycles();

	uint8_t rej = 0;
	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++) {
		poly_unpack(TC_L, parties[i].zi_1_val, _L, 0, z1_temp);
		poly_unpack(TC_L, parties[i].zi_2_val, _K, 0, z2_temp);

		for (size_t k = 0; k < TC_COLS; k++) { //WARNING !!!
			uint32_t nonce = 0;
			for (size_t j = 0; j < _K; j++) {
				do {
					nonce++;
					sample_normal_from_seed(parties[i].ri_val, j * TC_COLS + k + nonce, 0, TC_S, _N_, ri[j][k].coeffs);
				} while (!poly_check_norm(&ri[j][k], 1, TC_B));
			}
		}

		poly_center(z1_temp, _L);
		poly_center(z2_temp, _K);

		poly_add(z1, z1_temp, _L, z1);
		poly_add(z2, z2_temp, _K, z2);

		// w = Az1 - ct1 * 2^D
		poly_unpack(T1_BITS, parties[i].ti_val, _K, 0, t1_temp);

		// t1 * 2^D
		poly_const_mul(t1_temp, 1 << _D, _K, t1_temp);

		poly_ntt(z1_temp, _L);
		poly_ntt(t1_temp, _K);

		poly_product(A, z1_temp, w_temp);
		poly_mul_pointwise(t1_temp, &poly_c, _K, t1_temp);

		poly_reduce(w_temp, _K);
		poly_reduce(t1_temp, _K);

		poly_invntt_tomont(w_temp, _K);
		poly_invntt_tomont(t1_temp, _K);

		poly_add(w_temp, z2_temp, _K, w_temp);
		poly_sub(w_temp, t1_temp, _K, w_temp);

		poly_freeze(w_temp, _K);

		// com_i = ck*r + w
		poly_unpack(TC_L, parties[i].fi_commit, _K * _K, 0, (poly_t*)F2);

		poly_gen_commit2(ck, ri, F1);//WARNING !!!

		poly_add((poly_t*)&F1[1], (poly_t*)w_temp, _K, (poly_t*)&F1[1]);

		poly_freeze((poly_t*)F1, _K * _K);

		//check commitments
		rej = memcmp(F1, F2, sizeof(F1));

		if (rej != 0) {
			timestamp = get_cycles() - timestamp;//finish
			char errormsg[80] = { 0 };
			std::sprintf(errormsg, "Commitment check failed for node ID %d", i);
			err_code = DS2_ERROR_Fi_COMMIT;
			throw DS2Exception(DS2_ERROR_Fi_COMMIT, errormsg, timestamp);
		}
	}

	timestamp = get_cycles() - timestamp;//finish

	std::string sign_str = std::string((const char*)c, sizeof(c));

	sign_str += std::string((const char*)z1, sizeof(z1));

	sign_str += std::string((const char*)z2, sizeof(z2));

	for (int i = 0; i < DS2_MAX_PARTY_NUM; i++)
	{
		sign_str += std::string((const char*)r[i], sizeof(r[i]));
	}

	return sign_str;
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

	if (ready_flag)
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