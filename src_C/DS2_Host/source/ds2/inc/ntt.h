#ifndef DS2_NTT_H
#define DS2_NTT_H

#include <stdlib.h>
#include <stdint.h>
#include "params.h"

void ntt(int32_t a[_N_]);

void invntt_tomont(int32_t a[_N_]);

#endif
