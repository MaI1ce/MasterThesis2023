/*
 * stm_rng.c
 *
 *  Created on: Feb 19, 2024
 *      Author: 281154
 */

//do not forget about IRQ for RNG - void RNG_IRQHandler(void) in stm32wbxx_it.c

/*
 *
void RNG_IRQHandler(void)
{
  HAL_RNG_IRQHandler(&hrng);
}
 * */

#include "stm_rng.h"

#include "fips202.h"

RNG_HandleTypeDef hrng;

extern void Error_Handler(void);

static uint32_t master_seed;

#define RND_NUMBER_COUNT 8

static uint32_t index = 0;

static uint32_t rnd_numbers[RND_NUMBER_COUNT];

static keccak_state_t state;

void RNG_Init(void)
{
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }

  keccak_init(&state);

  while(HAL_RNG_GenerateRandomNumber(&hrng, &master_seed) != HAL_OK);

  shake256_absorb_nonce(&state, master_seed);

  shake256_finalize(&state);

  shake256_squeeze(&state, sizeof(uint32_t)*RND_NUMBER_COUNT, (uint8_t*)rnd_numbers);
}


HAL_StatusTypeDef RNG_GenerateRandomInt(uint32_t *rand_num)
{

	if(index < RND_NUMBER_COUNT){
		*rand_num = rnd_numbers[index];
		index++;
	} else {
		master_seed++;

		keccak_init(&state);
	    shake256_absorb_nonce(&state, master_seed);
	    shake256_finalize(&state);
	    shake256_squeeze(&state, sizeof(uint32_t)*RND_NUMBER_COUNT, (uint8_t*)rnd_numbers);

	    *rand_num = rnd_numbers[0];

	    index = 1;
	}
	//*rand_num = 0;
	return HAL_OK;
}

/**
* @brief RNG MSP Initialization
* This function configures the hardware resources used in this example
* @param hrng: RNG handle pointer
* @retval None
*/
void HAL_RNG_MspInit(RNG_HandleTypeDef* hrng)
{
	  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
	  RCC_OscInitStruct.PLL.PLLState= RCC_PLL_NONE;
	  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	  if (HAL_RCC_OscConfig(&RCC_OscInitStruct))
	  {
		  /* Initialization Error */
		  Error_Handler();
	  }
	  /*Select HSI48 output as RNG clock source */
	  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
	  PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;
	  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct))
	  {
	    /* Initialization Error */
		  Error_Handler();
	  }

	  /* RNG Peripheral clock enable */
	  __HAL_RCC_RNG_CLK_ENABLE();

}

/**
* @brief RNG MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hrng: RNG handle pointer
* @retval None
*/
void HAL_RNG_MspDeInit(RNG_HandleTypeDef* hrng)
{
  if(hrng->Instance==RNG)
  {
    /* Peripheral clock disable */
    __HAL_RCC_RNG_CLK_DISABLE();

    /* RNG interrupt DeInit */
    HAL_NVIC_DisableIRQ(RNG_IRQn);
  }

}
