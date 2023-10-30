/*
 * symmetric-aes_stm.h
 *
 *  Created on: Oct 28, 2023
 *      Author: MALICE
 */

#ifndef INC_SYMMETRIC_AES_STM_H_
#define INC_SYMMETRIC_AES_STM_H_

#include <stdint.h>
#include "stm32wbxx_hal.h"

#define BLOCKBYTES 64

typedef struct {
	  uint32_t ivw[4];
} stm_aes256ctr_ctx;

extern CRYP_HandleTypeDef hcryp_state;

HAL_StatusTypeDef dilithium_stm_stream_aes256ctr_init(stm_aes256ctr_ctx* state,
										const uint8_t key[32],
										uint16_t nonce);

HAL_StatusTypeDef dilithium_stm_stream_aes256ctr_deinit(stm_aes256ctr_ctx* state);

void dilithium_stm_stream_aes256ctr_squeezeblocks(uint8_t *out,
        										size_t nblocks,
												stm_aes256ctr_ctx* state);

void dilithium_stm_stream_aes256ctr_hash(stm_aes256ctr_ctx* state, uint8_t *out, size_t out_size, uint8_t *in, size_t in_size);

#endif /* INC_SYMMETRIC_AES_STM_H_ */
