#ifndef DS2_OSI3_H
#define DS2_OSI3_H

#include "stdint.h"
#include "ds2_osi3_conf.h"

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
	DS2_Ri_VALUE_ACK = DS2_Ri_VALUE |0x40,
	DS2_Zi_1_VALUE_ACK = DS2_Zi_1_VALUE | 0x40,
	DS2_Zi_2_VALUE_ACK = DS2_Zi_2_VALUE | 0x40,

	DS2_COORDINATOR_HELLO_ACK = DS2_COORDINATOR_HELLO | 0x40,

	DS2_KEYGEN_START_TASK = 0x80,
	DS2_SIGN_START_TASK ,
	DS2_VERIFY_START_TASK ,

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


typedef struct {
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

}DS2_Party;

typedef enum {
	DS2_IDLE = 0,
	DS2_READY,
	DS2_KEYGEN_START_IDLE,
	DS2_KEYGEN_START_END,
	DS2_KEYGEN_STAGE_1_IDLE,
	DS2_KEYGEN_STAGE_1_END,
	DS2_KEYGEN_STAGE_2_IDLE,
	DS2_KEYGEN_STAGE_2_END,
	DS2_KEYGEN_STAGE_3_IDLE,
	DS2_KEYGEN_STAGE_3_END,
	DS2_KEYGEN_FINAL_IDLE,
	DS2_KEYGEN_FINAL_END,
	DS2_SIGN_START_IDLE,
	DS2_SIGN_START_END,
	DS2_SIGN_STAGE_1_IDLE,
	DS2_SIGN_STAGE_1_END,
	DS2_SIGN_STAGE_2_IDLE,
	DS2_SIGN_STAGE_2_END,
	DS2_SIGN_STAGE_3_IDLE,
	DS2_SIGN_STAGE_3_END,
	DS2_SIGN_FINAL_IDLE,
	DS2_SIGN_FINAL_END,
	DS2_SIGN_REJECTED,
}DS2_State;

typedef struct {
	uint8_t packet_length;
	uint8_t src_node_id;
	uint8_t dst_node_id;
	uint8_t msg_code;
	uint32_t data_offset;
	uint32_t data[DS2_MAX_DATA_LEN];
} DS2_Packet;


#endif
