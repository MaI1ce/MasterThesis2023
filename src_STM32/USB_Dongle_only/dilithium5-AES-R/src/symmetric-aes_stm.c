/*
 * stm_aes256ctr.c
 *
 *  Created on: 28 окт. 2023 г.
 *      Author: MALICE
 */


#include "symmetric-aes_stm.h"

#define TIMEOUT_VALUE 0xFF

CRYP_HandleTypeDef hcryp_state;

HAL_StatusTypeDef dilithium_stm_stream_aes256ctr_init(stm_aes256ctr_ctx* state,
										const uint8_t key[32],
										uint16_t nonce)
{
	uint32_t n = nonce;
	state->ivw[0] = n;
	state->ivw[1] = n << 8;
	state->ivw[2] = n << 16;
	state->ivw[3] = (n << 24 )| (n >> 8 );

	uint32_t error = 0;
	if ((error = HAL_CRYP_DeInit(&hcryp_state)) != HAL_OK)
		return error;

	hcryp_state.Init.DataType  = CRYP_DATATYPE_1B;
	hcryp_state.Init.KeySize   = CRYP_KEYSIZE_256B;
	hcryp_state.Init.Algorithm = CRYP_AES_CTR;
	hcryp_state.Init.pKey      = (uint32_t *)key;
	hcryp_state.Init.pInitVect =  state->ivw;
	hcryp_state.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_WORD;
	hcryp_state.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ONCE;

	if ((error = HAL_CRYP_Init(&hcryp_state)) != HAL_OK)
		return error;

	return HAL_OK;
}

HAL_StatusTypeDef dilithium_stm_stream_aes256ctr_deinit(stm_aes256ctr_ctx* state)
{
	return HAL_CRYP_DeInit(&hcryp_state);
}


void dilithium_stm_stream_aes256ctr_squeezeblocks(uint8_t *out,
        										size_t nblocks,
												stm_aes256ctr_ctx* state)
{
	HAL_CRYP_Encrypt(&hcryp_state, (uint32_t *)out, nblocks*BLOCKBYTES/4, (uint32_t *)out, TIMEOUT_VALUE);
}


void dilithium_stm_stream_aes256ctr_hash(stm_aes256ctr_ctx* state, uint8_t *out, size_t out_size, uint8_t *in, size_t in_size)
{
	return;
}
