/*
 * usb_debug.c
 *
 *  Created on: 19 окт. 2023 г.
 *      Author: MALICE
 */
#include "usb_debug.h"

//uint8_t usb_msg_buf[80];

/*
Delay approx. 100 ns (93.75) at 64 MHz
Error is -6.25%
Test for 20 ms, takes 18.7 ms at 64 MHz
*/
void Delay_94ns(uint32_t lulDelay)
{
	while(lulDelay) // 2 cycles
	{
		--lulDelay; // 1 cycle
		asm("NOP"); // 1 cycles
		asm("NOP"); // 1 cycle
		asm("NOP"); // 1 cycle
	}
}

