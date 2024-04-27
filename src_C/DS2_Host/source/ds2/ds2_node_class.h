#ifndef DS2_NODE_H_
#define DS2_NODE_H_

#include <stdint.h>
#include <exception>
#include <string>
#include <cmath>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

#define MSG_SIZE 256
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

struct ds2_node {
public:
	uint32_t node_id;

	ds2_party self;

	std::string msg;

	uint32_t err_code;

	double coef;

	ds2_node() :
		node_id(0),
		coef(1),
		err_code(0),
		msg_buff{ 0 },
		rho{ 0 },
		t1{ 0 },
		t0{ 0 },
		A{ 0 },
		tr{ 0 },
		self(ds2_party()),
		c{ 0 },
		ck_seed{ 0 },
		poly_c{ 0 },
		z1{ 0 },
		z2{ 0 },
		s1{ 0 },
		s2{ 0 },
		y1{ 0 },
		y2{ 0 },
		ck{ 0 },
		ri{ 0 },
		w{ 0 },
		msg(std::string())
	{
		LARGE_INTEGER Frequency;
		QueryPerformanceFrequency(&Frequency);
		double f2 = double(Frequency.QuadPart);

		double f1 = 64000000.0;

		coef = f1 / f2;

		srand(time(NULL)); // init RND generator
	}

	std::string get_pi_commit();
	std::string get_pi_val();
	std::string get_ti_commit();
	std::string get_ti_val();

	void set_rho(const std::string& rho_);
	void set_tr(const std::string& tr_);
	void set_msg(const std::string& msg_);
	void set_c(const std::string& c_);

	std::string get_fi_commit();
	std::string get_zi_1_val();
	std::string get_zi_2_val();
	std::string get_ri_val();

	void reset();



	uint8_t msg_buff[MSG_SIZE];

	// Public key
	uint8_t		rho[SEED_BYTES];
	poly_t		A[_K_][_L_];
	uint8_t		tr[SEED_BYTES];
	poly_t		t1[_K_];
	poly_t 		t0[_K_];

	//private key
	uint8_t		private_seed[SEED_BYTES];
	poly_t		s1[_L_];
	poly_t		s2[_K_];

	// Signature
	uint8_t		c[SEED_BYTES];
	poly_t		poly_c;

	poly_t		y1[_L_];
	poly_t		y2[_K_];

	poly_t		z1[_L_];
	poly_t		z2[_K_];

	poly_t		w[_K_];

	poly_t		Fi[_K_][_K_];

	uint8_t		y_seed[SEED_BYTES];
	uint8_t		r_seed[DS2_Ri_VALUE_SIZE];
	uint8_t 	ck_seed[SEED_BYTES];
	poly_t		ck[_K_][TC_COLS];
	poly_t		ri[_K_][TC_COLS];

};


#endif