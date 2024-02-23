#ifndef DS2_OSI3_H
#define DS2_OSI3_H

#include "stdint.h"
#include "ds2_osi3_conf.h"

#define DS2_BROADCAST_ID 0xff

typedef enum {
	DS2_COORDINATOR_HELLO = 0,
	DS2_Pi_COMMIT,
	DS2_Pi_VALUE,
	DS2_Ti_COMMIT,
	DS2_Ti_VALUE,
	DS2_Fi_COMMIT,
	DS2_Ri_VALUE,
	DS2_Zi_VALUE,

	DS2_COORDINATOR_HELLO_ACK = DS2_COORDINATOR_HELLO | 0x40,
	DS2_Pi_COMMIT_ACK = DS2_Pi_COMMIT | 0x40,
	DS2_Pi_VALUE_ACK = DS2_Pi_VALUE | 0x40,
	DS2_Ti_COMMIT_ACK = DS2_Ti_COMMIT | 0x40,
	DS2_Ti_VALUE_ACK = DS2_Ti_VALUE | 0x40,
	DS2_Fi_COMMIT_ACK = DS2_Fi_COMMIT | 0x40,
	DS2_Ri_VALUE_ACK = DS2_Ri_VALUE |0x40,
	DS2_Zi_VALUE_ACK = DS2_Zi_VALUE | 0x40,

	DS2_ABORT = 0x80,
	DS2_ERROR_INVALID_NODE_ID = 0x1 | DS2_ABORT,
	DS2_ERROR_NODE_ID_ALREADY_IN_USE = 0x02 | DS2_ABORT,

	DS2_UNKNOWN_ERROR = 0xff
}ds2_msg_codes;

typedef struct {
	uint8_t packet_length;
	uint8_t src_node_id;
	uint8_t dst_node_id;
	uint8_t msg_code;
	uint32_t data_offset;
	uint32_t data[MAX_DATA_LEN];
} DS2_packet;


#endif
