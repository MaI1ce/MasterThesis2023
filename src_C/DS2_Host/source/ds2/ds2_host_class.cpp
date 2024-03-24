#include "ds2_host_class.h"

#include <cmath>

extern "C" {
#include "./inc/commit.h"
#include "./inc/fips202.h"
}

#include <string.h>

void ds2_host::reset()
{
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
	if (party_id >= party_num) 
		throw DS2Exception("party_id exceeds max allowed number of parties!");

	if (commit.size() != DS2_Pi_COMMIT_SIZE) {
		throw DS2Exception("pi_commit has bad buffer size");
	}
	else {
		memcpy(parties[party_id].pi_commit, commit.c_str(), DS2_Pi_COMMIT_SIZE);

		parties[party_id].status |= DS2_Pi_COMMIT_FLAG;
	}
}

void ds2_host::set_pi_val(uint32_t party_id, const std::string& val)
{
	uint8_t temp_commit[DS2_Pi_COMMIT_SIZE] = { 0 };

	if (party_id >= party_num)
		throw DS2Exception("party_id exceeds max allowed number of parties!");

	if (val.size() != DS2_Pi_COMMIT_SIZE) {
		throw DS2Exception("pi_val has bad buffer size");
	}
	else {
		memcpy(parties[party_id].pi_val, val.c_str(), DS2_Pi_COMMIT_SIZE);
		parties[party_id].status |= DS2_Pi_VALUE_FLAG;
	}

	h1(parties[party_id].pi_val, party_id, temp_commit);
	if (memcmp(temp_commit, parties[party_id].pi_commit, DS2_Pi_COMMIT_SIZE) != 0)
		throw DS2Exception("pi value do not correspond to provided pi_commit");

}

void ds2_host::set_ti_commit(uint32_t party_id, const std::string& commit)
{
	if (party_id >= party_num)
		throw DS2Exception("party_id exceeds max allowed number of parties!");

	if (commit.size() != DS2_Pi_COMMIT_SIZE) {
		throw DS2Exception("ti_commit has bad buffer size");
	}
	else {
		memcpy(parties[party_id].ti_commit, commit.c_str(), DS2_Ti_COMMIT_SIZE);

		parties[party_id].status |= DS2_Ti_COMMIT_FLAG;
	}
}

void ds2_host::set_ti_val(uint32_t party_id, const std::string& val)
{
	uint8_t temp_commit[DS2_Pi_COMMIT_SIZE] = { 0 };

	if (party_id >= party_num)
		throw DS2Exception("party_id exceeds max allowed number of parties!");

	if (val.size() != DS2_Ti_COMMIT_SIZE) {
		throw DS2Exception("ti_val has bad buffer size");
	}
	else {
		memcpy(parties[party_id].ti_val, val.c_str(), DS2_Ti_COMMIT_SIZE);
		parties[party_id].status |= DS2_Ti_VALUE_FLAG;
	}

	h1(parties[party_id].ti_val, party_id, temp_commit);
	if (memcmp(temp_commit, parties[party_id].ti_commit, DS2_Ti_COMMIT_SIZE) != 0)
		throw DS2Exception("pi value do not correspond to provided pi_commit");

}

void ds2_host::set_fi_commit(uint32_t party_id, const std::string& commit)
{
	if (party_id >= party_num)
		throw DS2Exception("party_id exceeds max allowed number of parties!");

	if (commit.size() != DS2_Fi_COMMIT_SIZE) {
		throw DS2Exception("fi_commit has bad buffer size");
	}
	else {
		memcpy(parties[party_id].fi_commit, commit.c_str(), DS2_Fi_COMMIT_SIZE);

		parties[party_id].status |= DS2_Fi_COMMIT_FLAG;
	}
}

void ds2_host::set_zi_1_val(uint32_t party_id, const std::string& val)
{
	if (party_id >= party_num)
		throw DS2Exception("party_id exceeds max allowed number of parties!");

	if (val.size() != DS2_Zi_1_VALUE_SIZE) {
		throw DS2Exception("zi_1_val has bad buffer size");
	}
	else {
		memcpy(parties[party_id].zi_1_val, val.c_str(), DS2_Zi_1_VALUE_SIZE);

		parties[party_id].status |= DS2_Zi_1_VALUE_FLAG;
	}
}


void ds2_host::set_zi_2_val(uint32_t party_id, const std::string& val)
{
	if (party_id >= party_num)
		throw DS2Exception("party_id exceeds max allowed number of parties!");

	if (val.size() != DS2_Zi_2_VALUE_SIZE) {
		throw DS2Exception("zi_2_val has bad buffer size");
	}
	else {
		memcpy(parties[party_id].zi_2_val, val.c_str(), DS2_Zi_2_VALUE_SIZE);

		parties[party_id].status |= DS2_Zi_2_VALUE_FLAG;
	}
}

void ds2_host::set_ri_val(uint32_t party_id, const std::string& val)
{

	if (party_id >= party_num)
		throw DS2Exception("party_id exceeds max allowed number of parties!");

	if (val.size() != DS2_Ri_VALUE_SIZE) {
		throw DS2Exception("ri_val has bad buffer size");
	}
	else {
		memcpy(parties[party_id].ri_val, val.c_str(), DS2_Ri_VALUE_SIZE);
		parties[party_id].status |= DS2_Ri_VALUE_FLAG;
	}

}