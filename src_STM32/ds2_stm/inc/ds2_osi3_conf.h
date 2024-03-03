#ifndef DS2_OSI3_CONF_H
#define DS2_OSI3_CONF_H

#include "params.h"

#define PARTY_NODE
//#define COORDINATOR_NODE

#define DS2_MAX_DATA_LEN 	25
#define DS2_MAX_PARTY_NUM 	1 // N_PARTIES
#define DS2_HEADER_LEN 		8
#define DS2_COORDINATOR_ID 	254
#define DS2_BROADCAST_ID 	0xff

#define DS2_Pi_COMMIT_SIZE 	L1
#define DS2_Pi_VALUE_SIZE 	SEED_BYTES

#define DS2_Ti_COMMIT_SIZE 	L2
#define DS2_Ti_VALUE_SIZE 	K*POLY_T1_PACKED_BYTES

#define DS2_Fi_COMMIT_SIZE	2 * 3 * K * _N

#define DS2_Zi_1_VALUE_SIZE 3 * L * _N
#define DS2_Zi_2_VALUE_SIZE 3 * K * _N

#define DS2_Ri_VALUE_SIZE 	SEED_BYTES

#ifdef PARTY_NODE
#define DS2_NODE_ID 0x0
#endif

#endif
