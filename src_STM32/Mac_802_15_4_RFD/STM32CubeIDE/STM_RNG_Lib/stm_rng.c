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

RNG_HandleTypeDef hrng;

extern void Error_Handler(void);

void RNG_Init(void)
{
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }

}


HAL_StatusTypeDef RNG_GenerateRandomInt(uint32_t *rand_num)
{
	return HAL_RNG_GenerateRandomNumber(&hrng, rand_num);
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
