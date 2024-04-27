#ifndef DS2_HOST_H_
#define DS2_HOST_H_

#include <stdint.h>
#include <exception>
#include <string>
#include <cmath>
#include <vector>
//#include <pybind11/pybind11.h>

#define __WINDOWS__
//#define __LINUX__

#ifdef __WINDOWS__
#include <windows.h>
#elif defined(__LINUX__)

#endif

extern "C" {
#include "./inc/params.h"
#include "./inc/poly.h"
#include "./inc/ds2_osi3_conf.h"
}

#include "ds2_setup.h"

class DS2Exception : public std::exception {
private:
	std::string err_msg;

public:
	uint64_t exec_time;
	uint32_t err_code;

	DS2Exception(uint32_t code, const char *msg): err_code(code), err_msg(msg), exec_time(0) {}

	DS2Exception(uint32_t code, const char* msg, uint64_t timestamp) : err_code(code),  err_msg(msg), exec_time(timestamp) {}


	const char* what() const noexcept {
		return err_msg.c_str();
	}
};

struct ds2_host {
public:
	uint32_t party_num; // = DS2_MAX_PARTY_NUM

	std::string msg;

	uint32_t err_code;

	double coef;

	ds2_host():
		coef(1),
		err_code(0),
		party_num(DS2_MAX_PARTY_NUM),
		rho{ 0 },
		t1{ 0 },
		A{ 0 },
		tr{ 0 },
		parties{ ds2_party() },
		c{ 0 },
		ck_seed{ 0 },
		poly_c{ 0 },
		z1{ 0 },
		z2{ 0 },
		ck{ 0 },
		msg(std::string())
	{
		LARGE_INTEGER Frequency;
		QueryPerformanceFrequency(&Frequency);
		double f2 = double(Frequency.QuadPart);

		double f1 = 64000000.0;

		coef = f1 / f2;
	}

	uint32_t get_party_num() { return party_num; }

	void set_pi_commit(uint32_t party_id, const std::string& commit);
	void set_pi_val(uint32_t party_id, const std::string& val);

	void set_ti_commit(uint32_t party_id, const std::string& commit);
	void set_ti_val(uint32_t party_id, const std::string& val);

	void set_fi_commit(uint32_t party_id, const std::string& commit);

	void set_zi_1_val(uint32_t party_id, const std::string& val);
	void set_zi_2_val(uint32_t party_id, const std::string& val);

	void set_ri_val(uint32_t party_id, const std::string& val);

	std::string get_rho(uint64_t& timestamp);

	std::string get_tr(uint64_t& timestamp);

	std::string hash_msg(const std::string& msg);

	std::string get_c(uint64_t& timestamp);

	std::string get_signature(uint64_t& timestamp);

	std::string get_publick_key();

	uint64_t get_timer();

	void set_public_key(const std::string& t1_, const std::string& rho_, const std::string& tr_);

	bool verify(const std::string& c_, const std::string& z1_, const std::string& z2_, const std::vector<std::string> ri_, uint64_t& timestamp);
	
	bool check_commit(const std::string& r, const std::string& ck, const std::string& fi, const std::string& wi, uint64_t& timestamp);

	bool check_commit2(const std::string& r, const std::string& ck, const std::string& fi, const std::string& wi, uint64_t& timestamp);

	bool is_flag_ready(uint32_t flag);
	void reset();

//private:
	ds2_party	parties[DS2_MAX_PARTY_NUM];

// Public key
	uint8_t		rho[SEED_BYTES];
	poly_t		t1[_K_];
	poly_t		A[_K_][_L_];
	uint8_t		tr[SEED_BYTES];

// Signature
	uint8_t		c[SEED_BYTES];
	poly_t		poly_c;
	poly_t		z1[_L_];
	poly_t		z2[_K_];
	uint8_t		r[DS2_MAX_PARTY_NUM][DS2_Ri_VALUE_SIZE];

// Current commitment key
	uint8_t 	ck_seed[SEED_BYTES];
	poly_t		ck[_K_][TC_COLS];
	poly_t		ri[_K_][TC_COLS];
};

#endif