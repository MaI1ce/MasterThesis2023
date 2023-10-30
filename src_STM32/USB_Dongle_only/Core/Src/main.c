/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
//#include <stdint.h>
//#include <string.h>
#include "fips202.h"
#include "params.h"
#include "sign.h"
#include "poly.h"
#include "polyvec.h"
#include "packing.h"

#include "usb_debug.h"

#include "elapsed_time.h"

#include "symmetric.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MLEN 59
#define NTESTS 5
//#define KEYGEN_TEST
//#define SIGN_VERIFY_TEST
#define FULL_TEST
//#define AES_TEST
//#define STATIC_KEYS
//#define KEYGEN
#if defined(STATIC_KEYS) || defined(SIGN_VERIFY_TEST)
#include "dilithium_keys.h"
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
//CRYP_HandleTypeDef hcryp1;
#ifdef HARDWARE_AES
__ALIGN_BEGIN static const uint32_t pKeyAES1[4] __ALIGN_END = {
                            0x00000000,0x00000000,0x00000000,0x00000000};
#endif

IPCC_HandleTypeDef hipcc;

RNG_HandleTypeDef hrng;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_IPCC_Init(void);
static void MX_RF_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_RNG_Init(void);
#ifdef HARDWARE_AES
static void MX_AES1_Init(void);
#endif
/* USER CODE BEGIN PFP */
static int sign_verify_test_assert(int ret, int mlen, uint8_t ref_m[MLEN], uint8_t new_m[MLEN]);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* IPCC initialisation */
  MX_IPCC_Init();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_RF_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_USB_Device_Init();
  MX_RNG_Init();
#ifdef HARDWARE_AES
  MX_AES1_Init();
#endif
  /* USER CODE BEGIN 2 */
  HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin); // BLUE LED
  HAL_Delay(3000);
  USB_DEBUG_MSG("Program Compilation Date: %s %s\n", __DATE__, __TIME__);
  USB_DEBUG_MSG("Dilithium Signature Test Start\n");
  HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);

#ifdef KEYGEN_TEST

  uint8_t pk[CRYPTO_PUBLICKEYBYTES];
  uint8_t sk[CRYPTO_SECRETKEYBYTES];

  elapsed_time_init();

#define TIME_STAMP_INDEX 0

  for (int i = 0; i < NTESTS; ++i) {
	  //
	  USB_DEBUG_MSG("Test %d\r\n", i);

	  elapsed_time_start(TIME_STAMP_INDEX);

	  crypto_sign_keypair(&hrng, pk, sk);

	  elapsed_time_stop(TIME_STAMP_INDEX);

	  USB_DEBUG_MSG("Cycles for keys generation max = %ld\r\n", elapsed_time_max(TIME_STAMP_INDEX));
	  USB_DEBUG_MSG("Cycles for keys generation min = %ld\r\n", elapsed_time_min(TIME_STAMP_INDEX));

	  elapsed_time_clr(TIME_STAMP_INDEX);
  }
  USB_DEBUG_MSG("CRYPTO_PUBLICKEYBYTES = %d\r\n", CRYPTO_PUBLICKEYBYTES);
  USB_DEBUG_MSG("CRYPTO_SECRETKEYBYTES = %d\r\n", CRYPTO_SECRETKEYBYTES);
  USB_DEBUG_MSG("CRYPTO_BYTES = %d\r\n", CRYPTO_BYTES);

#elif defined SIGN_VERIFY_TEST

  unsigned int i;
  int ret;
  size_t mlen, smlen;
  uint8_t m[MLEN] = { 0x55 };
  uint8_t sm[MLEN + CRYPTO_BYTES];
  uint8_t m2[MLEN + CRYPTO_BYTES];
  uint8_t test_ok = 1;

#define TIME_STAMP_SIGN 0
#define TIME_STAMP_VERF 1

  elapsed_time_init();

  for (i = 0; i < NTESTS; ++i) {
	  //
	  USB_DEBUG_MSG("Test %d\r\n", i);

	  elapsed_time_start(TIME_STAMP_SIGN );

	  crypto_sign(&hrng, sm, &smlen, m, MLEN, sk);

	  elapsed_time_stop(TIME_STAMP_SIGN );

	  elapsed_time_start(TIME_STAMP_VERF);

	  ret = crypto_sign_open(m2, &mlen, sm, smlen, pk);

	  elapsed_time_stop(TIME_STAMP_VERF);

	  if(sign_verify_test_assert(ret, mlen, m, m2)){
		  test_ok = 0;
		  break;
	  }

  }

  if (test_ok){
	  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin); // GREEN LED
	  USB_DEBUG_MSG("New Dilithium - OK\r\n");
  }
  else {
	  USB_DEBUG_MSG("New Dilithium - err_code = %d\r\n", ret);
  }

  USB_DEBUG_MSG("Cycles for full signature generation max = %ld\r\n", elapsed_time_max(TIME_STAMP_SIGN));
  USB_DEBUG_MSG("Cycles for full signature generation min = %ld\r\n", elapsed_time_min(TIME_STAMP_SIGN));
  USB_DEBUG_MSG("Cycles for full signature verification max = %ld\r\n", elapsed_time_max(TIME_STAMP_VERF));
  USB_DEBUG_MSG("Cycles for full signature verification min = %ld\r\n", elapsed_time_min(TIME_STAMP_VERF));
  USB_DEBUG_MSG("CRYPTO_PUBLICKEYBYTES = %d\r\n", CRYPTO_PUBLICKEYBYTES);
  USB_DEBUG_MSG("CRYPTO_SECRETKEYBYTES = %d\r\n", CRYPTO_SECRETKEYBYTES);
  USB_DEBUG_MSG("CRYPTO_BYTES = %d\r\n", CRYPTO_BYTES);

#elif defined FULL_TEST
  unsigned int i, j;
  int ret;
  size_t mlen, smlen;
  uint8_t m[MLEN] = { 0x55 };
  uint8_t sm[MLEN + CRYPTO_BYTES];
  uint8_t m2[MLEN + CRYPTO_BYTES];
  uint8_t test_ok = 1;
  uint8_t pk[CRYPTO_PUBLICKEYBYTES];
  uint8_t sk[CRYPTO_SECRETKEYBYTES];

  for (i = 0; i < NTESTS; ++i) {
	  //
	  USB_DEBUG_MSG("Test %d\r\n", i);

	  crypto_sign_keypair(&hrng, pk, sk);
	  crypto_sign(&hrng, sm, &smlen, m, MLEN, sk);
	  ret = crypto_sign_open(m2, &mlen, sm, smlen, pk);

	  if(sign_verify_test_assert(ret, mlen, m, m2)){
		  test_ok = 0;
		  break;
	  }

  }

  if (test_ok){
	  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin); // GREEN LED
	  USB_DEBUG_MSG("New Dilithium - OK\r\n");
  }
  else {
	  USB_DEBUG_MSG("New Dilithium - err_code = %d\r\n", ret);
  }

#endif


#ifdef TIME_BENCHMARK
  USB_DEBUG_MSG("TIME BENCHMARK FOR %d TESTS\r\n",NTESTS);
#if defined(STATIC_KEYS) || defined(SIGN_VERIFY_TEST)
  USB_DEBUG_MSG("Cycles for keygen t = A*s1+s2 max = %ld\r\n", elapsed_time_max(KEYS_SMUL_TIMESTAMP));
  USB_DEBUG_MSG("Cycles for keygen t = A*s1+s2 min = %ld\r\n", elapsed_time_min(KEYS_SMUL_TIMESTAMP));
#else
  USB_DEBUG_MSG("PREINSTALLED KEYS USED\r\n");
#endif
  USB_DEBUG_MSG("Cycles for sign   w = Ay      max = %ld\r\n", elapsed_time_max(SIGN_YMUL_TIMESTAMP));
  USB_DEBUG_MSG("Cycles for sign   w = Ay      min = %ld\r\n", elapsed_time_min(SIGN_YMUL_TIMESTAMP));
  USB_DEBUG_MSG("Cycles for sign   z = y+c*s1  max = %ld\r\n", elapsed_time_max(SIGN_Z_COMPUTE_TIMESTAMP));
  USB_DEBUG_MSG("Cycles for sign   z = y+c*s1  min = %ld\r\n", elapsed_time_min(SIGN_Z_COMPUTE_TIMESTAMP));
  USB_DEBUG_MSG("Cycles for sign   h0 = c*s2   max = %ld\r\n", elapsed_time_max(SIGN_H_COMPUTE_TIMESTAMP));
  USB_DEBUG_MSG("Cycles for sign   h0 = c*s2   min = %ld\r\n", elapsed_time_min(SIGN_H_COMPUTE_TIMESTAMP));
  USB_DEBUG_MSG("Cycles for verify w1 = Usehint(Az-c*t1*2^d) max = %ld\r\n", elapsed_time_max(VERIFY_RECONSTRUC_W1_TIMESTAMP));
  USB_DEBUG_MSG("Cycles for verify w1 = Usehint(Az-c*t1*2^d) min = %ld\r\n", elapsed_time_min(VERIFY_RECONSTRUC_W1_TIMESTAMP));
#endif

#ifdef AES_TEST
#define POLY_UNIFORM_NBLOCKS ((768 + STREAM128_BLOCKBYTES - 1)/STREAM128_BLOCKBYTES)
  uint8_t buf[POLY_UNIFORM_NBLOCKS*STREAM128_BLOCKBYTES + 2] = {0};
  stream128_state state;
  uint8_t seed[32] = {0x55};
  int nonce = 0;

  poly a;

  for(int i = 0; i < NTESTS; i++, nonce++){
	  //randombytes(&hrng, seed, 32);
	  USB_DEBUG_MSG("\r\nTETS %d\r\n", i);

	  USB_DEBUG_MSG(" first\r\n");
	  USB_DEBUG_MSG(" seed =");
	  for(int j = 0; j < 32; j++){
		  USB_DEBUG_MSG("%d ", seed[j]);
	  }
	  USB_DEBUG_MSG("\r\n");

	  poly_uniform(&a, seed, 1);


	  //for(int k = 0; k < K; k++){
		  //for(int l = 0; l < L; l++){
			  USB_DEBUG_MSG("a = ");
			  for(int j = 0; j < N_; j++){
				  USB_DEBUG_MSG("%ld", a.coeffs[j]);
			  }
			  USB_DEBUG_MSG("\r\n");
		  //}
		  //USB_DEBUG_MSG("\r\n");
	  //}

	  USB_DEBUG_MSG("\r\n second\r\n");
	  USB_DEBUG_MSG(" seed =");
	  for(int j = 0; j < 32; j++){
		  USB_DEBUG_MSG("%d ", seed[j]);
	  }
	  USB_DEBUG_MSG("\r\n");

	  poly_uniform(&a, seed, 1);


	  //for(int k = 0; k < K; k++){
		  //for(int l = 0; l < L; l++){
			  USB_DEBUG_MSG("a = ");
			  for(int j = 0; j < N_; j++){
				  USB_DEBUG_MSG("%ld", a.coeffs[j]);
			  }
			  USB_DEBUG_MSG("\r\n");
		  //}
		  //USB_DEBUG_MSG("\r\n");
	  //}
  }





#endif

#ifdef KEYGEN
  uint8_t pk[CRYPTO_PUBLICKEYBYTES];
  uint8_t sk[CRYPTO_SECRETKEYBYTES];

  crypto_sign_keypair(&hrng, pk, sk);

  USB_DEBUG_MSG("Public key:\r\n");
  for (int i = 0; i < sizeof(pk); i++) {
      if (i % 8 == 0)
    	  USB_DEBUG_MSG("\n\t");
      USB_DEBUG_MSG("0x%02x, ", pk[i]);
  }

  USB_DEBUG_MSG("Secret key:\r\n");
  for (int i = 0; i < sizeof(sk); i++) {
      if (i % 8 == 0)
    	  USB_DEBUG_MSG("\n\t");
      USB_DEBUG_MSG("0x%02x, ", sk[i]);
  }
#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	  HAL_Delay(1000);
	  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); // YELLOW LED
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI
                              |RCC_OSCILLATORTYPE_LSI1|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS|RCC_PERIPHCLK_RFWAKEUP;
  PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_HSE_DIV1024;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/**
  * @brief AES1 Initialization Function
  * @param None
  * @retval None
  */

#ifdef HARDWARE_AES
static void MX_AES1_Init(void)
{

  /* USER CODE BEGIN AES1_Init 0 */

  /* USER CODE END AES1_Init 0 */

  /* USER CODE BEGIN AES1_Init 1 */

  /* USER CODE END AES1_Init 1 */
	hcryp_state.Instance = AES1;
	hcryp_state.Init.DataType = CRYP_DATATYPE_32B;
	hcryp_state.Init.KeySize = CRYP_KEYSIZE_128B;
	hcryp_state.Init.pKey = (uint32_t *)pKeyAES1;
	hcryp_state.Init.Algorithm = CRYP_AES_ECB;
	hcryp_state.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_WORD;
	hcryp_state.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ALWAYS;
  if (HAL_CRYP_Init(&hcryp_state) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN AES1_Init 2 */

  /* USER CODE END AES1_Init 2 */

}
#endif

/**
  * @brief IPCC Initialization Function
  * @param None
  * @retval None
  */
static void MX_IPCC_Init(void)
{

  /* USER CODE BEGIN IPCC_Init 0 */

  /* USER CODE END IPCC_Init 0 */

  /* USER CODE BEGIN IPCC_Init 1 */

  /* USER CODE END IPCC_Init 1 */
  hipcc.Instance = IPCC;
  if (HAL_IPCC_Init(&hipcc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IPCC_Init 2 */

  /* USER CODE END IPCC_Init 2 */

}

/**
  * @brief RF Initialization Function
  * @param None
  * @retval None
  */
static void MX_RF_Init(void)
{

  /* USER CODE BEGIN RF_Init 0 */

  /* USER CODE END RF_Init 0 */

  /* USER CODE BEGIN RF_Init 1 */

  /* USER CODE END RF_Init 1 */
  /* USER CODE BEGIN RF_Init 2 */

  /* USER CODE END RF_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : EXTI10_Pin */
  GPIO_InitStruct.Pin = EXTI10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(EXTI10_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin LED2_Pin LED3_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LED4_Pin */
  GPIO_InitStruct.Pin = LED4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED4_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
static int sign_verify_test_assert(int ret, int mlen, uint8_t ref_m[MLEN], uint8_t new_m[MLEN]){
	  if (ret) {
	 		  USB_DEBUG_MSG("Verification failed - err_code = %d\r\n", ret);
	 		  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET); // RED LED
	 		 return 1;
	 }

	  if (mlen != MLEN) {
		  USB_DEBUG_MSG("Message lengths don't match\r\n");
		  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET); // RED LED
		  return 1;
	  }

	  for (int j = 0; j < mlen; ++j) {
		  if (ref_m[j] != new_m[j]) {
			  USB_DEBUG_MSG("Messages don't match\r\n");
			  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET); // RED LED
			  return 1;
		  }
	  }
	  return 0;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	  HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
	  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
	  HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
	  HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
	  HAL_Delay(500);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
