#include <stdlib.h>

#include "ds2/params.h"
#include "ds2/reduce.h"

int32_t reduce32(int32_t a) {
  int32_t t = (a + (1 << 22)) >> 23;
  t = a - t * Q;

  return t;
}

int32_t addq(int32_t a) {
  a += (a >> 31) & Q;

  return a;
}

int32_t freeze(int32_t a) {
  a = reduce32(a);
  a = addq(a);

  return a;
}

int32_t center(int32_t a) {
  a = freeze(a);

  return a > (Q >> 1) ? a - Q : a;
}

int32_t montgomery_reduce(int64_t a) {
  int32_t t = a * Q_INV;
  t = (a - (int64_t) t * Q) >> 32;

  return t;
}