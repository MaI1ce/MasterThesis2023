/*
********************************************************************************
*                       MODULE TO MEASURE EXECUTION TIME
********************************************************************************
*/

/*
********************************************************************************
*                MAXIMUM NUMBER OF ELAPSED TIME MEASUREMENT SECTIONS
********************************************************************************
*/
#ifndef ELAPSED_TIME_H_
#define ELAPSED_TIME_H_

#define  ELAPSED_TIME_MAX_SECTIONS  10

#define TB_START(i) elapsed_time_start((i))
#define TB_END(i) elapsed_time_stop((i))

/*
********************************************************************************
*                             FUNCTION PROTOTYPES
********************************************************************************
*/

void  elapsed_time_clr   (uint32_t  i);      // Clear measured values
void  elapsed_time_init  (void);             // Module initialization
void  elapsed_time_start (uint32_t  i);      // Start measurement 
void  elapsed_time_stop  (uint32_t  i);      // Stop  measurement 
uint32_t elapsed_time_max(uint32_t  i);
uint32_t elapsed_time_min(uint32_t  i);

#endif
