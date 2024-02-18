#ifndef DS2_REDUCE_H
#define DS2_REDUCE_H

#include <stdint.h>
#include <stdlib.h>

#define Q_INV 58728449

int32_t reduce32(int32_t a);

int32_t addq(int32_t a);

int32_t freeze(int32_t a);

int32_t center(int32_t a);

int32_t montgomery_reduce(int64_t a);

#endif