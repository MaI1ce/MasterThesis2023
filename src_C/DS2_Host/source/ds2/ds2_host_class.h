#ifndef DS2_HOST_H_
#define DS2_HOST_H_

#include <stdint.h>
#include <exception>
#include <string>
#include <pybind11/pybind11.h>

extern "C" {
#include "./inc/params.h"
#include "./inc/poly.h"
#include "./inc/ds2_osi3_conf.h"
}

class DS2Exception : public std::exception {
private:
	std::string err_msg;
public:
	DS2Exception(const char *msg): err_msg(msg) {}


	const char* what() const noexcept {
		return err_msg.c_str();
	}
};


typedef enum {
	DS2_Pi_COMMIT = 0,
	DS2_Pi_VALUE,
	DS2_Ti_COMMIT,
	DS2_Ti_VALUE,
	DS2_Fi_COMMIT,
	DS2_Ri_VALUE,
	DS2_Zi_1_VALUE,
	DS2_Zi_2_VALUE,

	DS2_COORDINATOR_HELLO,
	DS2_COORDINATOR_READY_RESET,

	DS2_Pi_COMMIT_ACK = DS2_Pi_COMMIT | 0x40,
	DS2_Pi_VALUE_ACK = DS2_Pi_VALUE | 0x40,
	DS2_Ti_COMMIT_ACK = DS2_Ti_COMMIT | 0x40,
	DS2_Ti_VALUE_ACK = DS2_Ti_VALUE | 0x40,
	DS2_Fi_COMMIT_ACK = DS2_Fi_COMMIT | 0x40,
	DS2_Ri_VALUE_ACK = DS2_Ri_VALUE | 0x40,
	DS2_Zi_1_VALUE_ACK = DS2_Zi_1_VALUE | 0x40,
	DS2_Zi_2_VALUE_ACK = DS2_Zi_2_VALUE | 0x40,

	DS2_COORDINATOR_HELLO_ACK = DS2_COORDINATOR_HELLO | 0x40,

	DS2_KEYGEN_START_TASK = 0x80,
	DS2_SIGN_START_TASK,
	DS2_VERIFY_START_TASK,

	DS2_ABORT = 0xC0,
	DS2_ERROR_INVALID_NODE_ID = 0x1 | DS2_ABORT,
	DS2_ERROR_NODE_ID_ALREADY_IN_USE = 0x02 | DS2_ABORT,
	DS2_ERROR_Pi_COMMIT = 0x03 | DS2_ABORT,
	DS2_ERROR_Ti_COMMIT = 0x04 | DS2_ABORT,
	DS2_ERROR_Fi_COMMIT = 0x05 | DS2_ABORT,
	DS2_ERROR_Zi_REJECT = 0X06 | DS2_ABORT,

	DS2_UNKNOWN_ERROR = 0xff
}DS2_msg_codes;

#define DS2_Pi_COMMIT_FLAG 		(1U << DS2_Pi_COMMIT)
#define DS2_Pi_VALUE_FLAG 		(1U << DS2_Pi_VALUE)
#define DS2_Ti_COMMIT_FLAG 		(1U << DS2_Ti_COMMIT)
#define DS2_Ti_VALUE_FLAG 		(1U << DS2_Ti_VALUE)
#define DS2_Fi_COMMIT_FLAG 		(1U << DS2_Fi_COMMIT)
#define DS2_Ri_VALUE_FLAG 		(1U << DS2_Ri_VALUE)
#define DS2_Zi_1_VALUE_FLAG 	(1U << DS2_Zi_1_VALUE)
#define DS2_Zi_2_VALUE_FLAG 	(1U << DS2_Zi_2_VALUE)
#define DS2_PARTY_ACTIVE   		0x80000000

struct ds2_party {
public:
	uint32_t status;

	/*
	* bit 0 = 1 - DS2_Pi_COMMIT 	received by coordinator
	* bit 1 = 1 - DS2_Pi_VALUE  	received by coordinator
	* bit 2 = 1 - DS2_Ti_COMMIT 	received by coordinator
	* bit 3 = 1 - DS2_Ti_VALUE  	received by coordinator
	* bit 4 = 1 - DS2_Fi_COMMIT 	received by coordinator
	* bit 5 = 1 - DS2_Ri_VALUE  	received by coordinator
	* bit 6 = 1 - DS2_Zi_1_VALUE  	received by coordinator
	* bit 7 = 1 - DS2_Zi_2_VALUE  	received by coordinator
	* bit 31 = 1 - party is active and associated with coordinator
	*/

	uint8_t pi_commit[DS2_Pi_COMMIT_SIZE];
	uint8_t pi_val[DS2_Pi_VALUE_SIZE];

	uint8_t ti_commit[DS2_Ti_COMMIT_SIZE];
	uint8_t ti_val[DS2_Ti_VALUE_SIZE];


	uint8_t fi_commit[DS2_Fi_COMMIT_SIZE];

	uint8_t zi_1_val[DS2_Zi_1_VALUE_SIZE];
	uint8_t zi_2_val[DS2_Zi_2_VALUE_SIZE];

	uint8_t ri_val[DS2_Ri_VALUE_SIZE];

	ds2_party() :
		status(0),
		pi_commit{ 0 },
		pi_val{ 0 },
		ti_commit{ 0 },
		ti_val{ 0 },
		fi_commit{ 0 },
		zi_1_val{ 0 },
		zi_2_val{ 0 },
		ri_val{ 0 }
	{}
};

struct ds2_host {
public:
	uint32_t party_num; // = DS2_MAX_PARTY_NUM

	ds2_host():
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
		ck{ 0 }
	{}

	uint32_t get_party_num() { return party_num; }

	void set_pi_commit(uint32_t party_id, const std::string& commit);
	void set_pi_val(uint32_t party_id, const std::string& val);

	void set_ti_commit(uint32_t party_id, const std::string& commit);
	void set_ti_val(uint32_t party_id, const std::string& val);

	void set_fi_commit(uint32_t party_id, const std::string& commit);

	void set_zi_1_val(uint32_t party_id, const std::string& val);
	void set_zi_2_val(uint32_t party_id, const std::string& val);

	void set_ri_val(uint32_t party_id, const std::string& val);

	bool is_flag_ready(uint32_t flag);
	void reset();

private:
	ds2_party	parties[DS2_MAX_PARTY_NUM];

// Public key
	uint8_t		rho[SEED_BYTES];
	poly_t		t1[K];
	poly_t		A[K][L];
	uint8_t		tr[SEED_BYTES];

// Signature
	uint8_t		c[SEED_BYTES];
	poly_t		poly_c;
	poly_t		z1[L];
	poly_t		z2[K];
	uint8_t		r[DS2_MAX_PARTY_NUM][DS2_Ri_VALUE_SIZE];

// Current commitment key
	uint8_t 	ck_seed[SEED_BYTES];
	poly_t		ck[K][TC_COLS];
};

#endif