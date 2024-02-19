/*
 * stm_rng.h
 *
 *  Created on: Feb 19, 2024
 *      Author: 281154
 */

#ifndef STM_RNG_H_
#define STM_RNG_H_

#include "stm32wbxx_hal.h"
#include "stdint.h"

extern RNG_HandleTypeDef hrng;

void RNG_Init(void);

HAL_StatusTypeDef RNG_GenerateRandomInt(uint32_t *rand_num);


#endif /* STM_RNG_H_ */
