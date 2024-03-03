#include "reduce.h"

#include <stdlib.h>

#include "params.h"

int32_t reduce32(int32_t a) {
  int32_t t = (a + (1 << 22)) >> 23;
  t = a - t * _Q;

  return t;
}

int32_t addq(int32_t a) {
  a += (a >> 31) & _Q;

  return a;
}

int32_t freeze(int32_t a) {
  a = reduce32(a);
  a = addq(a);

  return a;
}

int32_t center(int32_t a) {
  a = freeze(a);

  return a > (_Q >> 1) ? a - _Q : a;
}

int32_t montgomery_reduce(int64_t a) {
  int32_t t = a * Q_INV;
  t = (a - (int64_t) t * _Q) >> 32;

  return t;
}
