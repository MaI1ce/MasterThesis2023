/*
 * time_testbench.h
 *
 *  Created on: 22 окт. 2023 г.
 *      Author: MALICE
 */

#ifndef INC_TIME_TESTBENCH_H_
#define INC_TIME_TESTBENCH_H_

extern long unsigned int cpu_cycle_counter;
#define DWT_STOPWATCH_RESET() cpu_cycle_counter = 0

#if defined STM32WB55xx

#define DWT_STOPWATCH_START() DWT->CYCCNT = 0; DWT->CTRL |= 1 // reset and enable clock counter, see RM0434

#define DWT_STOPWATCH_LATCH(t) t += DWT->CYCCNT; DWT->CTRL &= ~(0x1)

#else
#define DWT_STOPWATCH_START()
#define DWT_STOPWATCH_LATCH(t)
#endif

#endif /* INC_TIME_TESTBENCH_H_ */
