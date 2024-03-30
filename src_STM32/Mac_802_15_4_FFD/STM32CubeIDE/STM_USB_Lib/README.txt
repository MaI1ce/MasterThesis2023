1) Enable PCD in stm32wbxx_hal_conf.h
2) copy Interrupts in stm32wbxx_it.c

extern PCD_HandleTypeDef hpcd_USB_FS;

/**
  * @brief This function handles USB high priority interrupt.
  */
void USB_HP_IRQHandler(void)
{
  /* USER CODE BEGIN USB_HP_IRQn 0 */

  /* USER CODE END USB_HP_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
  /* USER CODE BEGIN USB_HP_IRQn 1 */

  /* USER CODE END USB_HP_IRQn 1 */
}

/**
  * @brief This function handles USB low priority interrupt, USB wake-up interrupt through EXTI line 28.
  */
void USB_LP_IRQHandler(void)
{
  /* USER CODE BEGIN USB_LP_IRQn 0 */

  /* USER CODE END USB_LP_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
  /* USER CODE BEGIN USB_LP_IRQn 1 */

  /* USER CODE END USB_LP_IRQn 1 */
}

3) in clock setup enable HSI48 in main.c

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;

4) include in main.c

#include "usb_device.h"
#include "usbd_cdc_if.h"

5) Init USB - MX_USB_Device_Init();

define 

static int8_t CDC_Receive_FS (uint8_t* Buf, uint32_t *Len)
{
    // Process Len bytes from Buf
    YOUR_CODE_HERE

    // Set the RX buffer
    USBD_CDC_SetRxBuffer(hUsbDevice_0, &Buf[0]);
    
    // Ready to receive the next packet
    USBD_CDC_ReceivePacket(hUsbDevice_0);

    return USBD_OK ;
}