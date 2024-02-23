#ifndef DS2_OSI3_H
#define DS2_OSI3_H

#include "ds2_osi3_conf.h"
#include "stdint.h"

typedef enum {
	DS2_Pi_COMMIT = 0,
	DS2_Pi_VALUE = 1,
	DS2_Ti_COMMIT = 2,
	DS2_Ti_VALUE = 3,
	DS2_Fi_COMMIT = 4,
	DS2_Ri_VALUE = 5,
	DS2_Zi_VALUE = 6,
	DS2_Pi_COMMIT_ACK = DS2_Pi_COMMIT | 0x80,
	DS2_Pi_VALUE_ACK = DS2_Pi_VALUE | 0x80,
	DS2_Ti_COMMIT_ACK = DS2_Ti_COMMIT | 0x80,
	DS2_Ti_VALUE_ACK = DS2_Ti_VALUE | 0x80,
	DS2_Fi_COMMIT_ACK = DS2_Fi_COMMIT | 0x80,
	DS2_Ri_VALUE_ACK = DS2_Ri_VALUE |0x80,
	DS2_Zi_VALUE_ACK = DS2_Zi_VALUE | 0x80,
	DS2_ABORT = 0xff
}ds2_msg_codes;

typedef struct {
	uint8_t packet_length;
	uint8_t src_node_id;
	uint8_t dst_node_id;
	uint8_t msg_code;
	uint32_t data_offset;
	uint32 data[MAX_DATA_LEN];
} DS2_packet;


#endif