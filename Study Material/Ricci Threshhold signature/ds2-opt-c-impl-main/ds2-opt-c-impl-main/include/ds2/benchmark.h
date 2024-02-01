#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdio.h>
#include <time.h>

extern clock_t measured_clocks[10];
extern size_t run_numbers[10];

#define EXPAND_A_INDEX 0
#define KEYPAIR_INDEX 1
#define POLY_UNIFORM_INDEX 2
#define POLY_COMMIT_INDEX 3
#define POLY_OPEN_INDEX 4
#define SIGN_ITERATION_INDEX 5

#define BENCHMARK

#ifdef BENCHMARK
#define TB_START(i) clock_t start_##i = clock();
#define TB_END(i) measured_clocks[i] += clock() - start_##i; \
    run_numbers[i] += 1;
#else
#define TB_START(i)
#define TB_END(i)
#endif

#define AVERAGE_MS(x, n) (1000.0 * x / (n * CLOCKS_PER_SEC))

void print_results();

#endif