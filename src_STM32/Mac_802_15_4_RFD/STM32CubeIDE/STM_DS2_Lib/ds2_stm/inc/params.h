#ifndef DS2_PARAMS_H
#define DS2_PARAMS_H

// #define USE_SAFE_RANDOM

#define N_PARTIES 2

#define _N 256
#define _Q 8380417

#define K 4
#define L 4

#define D 12

#define ETA 5
#define GAMMA 2 // gamma > 1
#define KAPPA 60 // maximum L^1-norm of challenge vector c

#if N_PARTIES == 2
#define N_PARTIES_SQRT 1.44

#define B 27034695 // maximum L^2-norm of signature z (B = gamma * sigma * sqrt((l + k) * N))
#define M 1.7272 // e^(t/alpha+1/(2alpha^2))
#elif N_PARTIES == 5
#define N_PARTIES_SQRT 2.24

#define B 67586737 // maximum L^2-norm of signature z (B = gamma * sigma * sqrt((l + k) * N))
#define M 1.2440 // e^(t/alpha+1/(2alpha^2))
#elif N_PARTIES == 10
#define N_PARTIES_SQRT 3.16

#define B 135173474 // maximum L^2-norm of signature z (B = gamma * sigma * sqrt((l + k) * N))
#define M 1.1153 // e^(t/alpha+1/(2alpha^2))
#elif N_PARTIES == 20
#define N_PARTIES_SQRT 4.47

#define B 270346948 // maximum L^2-norm of signature z (B = gamma * sigma * sqrt((l + k) * N))
#define M 1.0561 // e^(t/alpha+1/(2alpha^2))
#elif N_PARTIES == 50
#define N_PARTIES_SQRT 7.07

#define B 675867370 // maximum L^2-norm of signature z (B = gamma * sigma * sqrt((l + k) * N))
#define M 1.0221 // e^(t/alpha+1/(2alpha^2))
#endif

// TODO: why N_PARTIES? can't find it anywhere in paper
// #define SIGMA (_alpha * N_PARTIES * T) // alpha * n * T
#define SIGMA (_alpha  * _T) // alpha * n * T

#define _T 13577 // kappa * eta * sqrt(N (l + k))

#define ALPHA 16368

#define __t 12 // ensuring > 100 bits of security
#define _alpha 11 // 17 -> M = 2, 29 -> M = 1.5, 53 -> 1.25

#define L1 64
#define L2 64

#define SEED_BYTES 64
#define T1_BITS 11
#define POLY_T1_PACKED_BYTES (T1_BITS * (_N >> 3))

#define TC_S_ 2059
#define TC_S 815
#define TC_B 48415
#define TC_L 23 // ceil(log2(q))
#define TC_W TC_L
#define TC_COLS (TC_L + 2 * TC_W)

#endif
