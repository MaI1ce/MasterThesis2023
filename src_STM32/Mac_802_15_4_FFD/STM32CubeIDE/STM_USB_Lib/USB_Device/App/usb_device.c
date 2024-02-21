/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_device.c
  * @version        : v3.0_Cube
  * @brief          : This file implements the USB Device
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"

/* USER CODE BEGIN Includes */
#include "stm_logging.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

extern void Error_Handler(void);
/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_DescriptorsTypeDef CDC_Desc;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_Device_Init(void)
{
  /* USER CODE BEGIN USB_Device_Init_PreTreatment */
  uint8_t status = USBD_OK;
  LL_HSEM_1StepLock( HSEM, 5 );
  /* USER CODE END USB_Device_Init_PreTreatment */

  /* Init Device Library, add supported class and start the library. */
  if ((status = USBD_Init(&hUsbDeviceFS, &CDC_Desc, DEVICE_FS)) != USBD_OK) {
	  APP_DBG("ERROR USBD_Init - %ld", status);
    Error_Handler();
  }
  if ((status = USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC)) != USBD_OK) {
	  APP_DBG("ERROR USBD_RegisterClass - %ld", status);
    Error_Handler();
  }
  if ((status = USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS)) != USBD_OK) {
	  APP_DBG("ERROR USBD_CDC_RegisterInterface - %ld", status);
    Error_Handler();
  }
  if ((status = USBD_Start(&hUsbDeviceFS)) != USBD_OK) {
	  APP_DBG("ERROR USBD_Start - %ld", status);
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Device_Init_PostTreatment */

  /* USER CODE END USB_Device_Init_PostTreatment */
}

/**
  * @}
  */

/**
  * @}
  */

