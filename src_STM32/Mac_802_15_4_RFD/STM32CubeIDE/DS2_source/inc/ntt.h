#ifndef DS2_NTT_H
#define DS2_NTT_H

#include <stdlib.h>

#include "ds2/params.h"

void ntt(int32_t a[N]);

void invntt_tomont(int32_t a[N]);

#endif
