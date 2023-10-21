/*
 * usb_debug.h
 *
 *  Created on: 19 окт. 2023 г.
 *      Author: MALICE
 */

#ifndef INC_USB_DEBUG_H_
#define INC_USB_DEBUG_H_

#include "stdio.h"
#include "stdint.h"
#include "string.h"

#ifndef __USB_DEVICE__H__
//#include "usb_device.h"
#include "usbd_cdc_if.h"
#endif

extern uint8_t usb_msg_buf[80];

void Delay_100ns(uint32_t lulDelay);

#define USB_DEBUG_MSG(...) do {\
		memset(usb_msg_buf, 0, 80);\
		sprintf ((char*)usb_msg_buf, __VA_ARGS__);\
		CDC_Transmit_FS((uint8_t*)usb_msg_buf, strlen((const char*)usb_msg_buf));\
		Delay_100ns(1000);\
	} while (0)


#endif /* INC_USB_DEBUG_H_ */
