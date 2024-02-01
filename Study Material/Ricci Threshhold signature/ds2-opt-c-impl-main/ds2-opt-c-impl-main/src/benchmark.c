#include "ds2/benchmark.h"

clock_t measured_clocks[10] = {};
size_t run_numbers[10] = {};

void print_results() {
    printf("generate_matrix: %f ms\n", AVERAGE_MS(measured_clocks[EXPAND_A_INDEX], run_numbers[EXPAND_A_INDEX]));
    printf("generate_key_pair: %f ms\n", AVERAGE_MS(measured_clocks[KEYPAIR_INDEX], run_numbers[KEYPAIR_INDEX]));
    printf("poly_uniform: %f ms\n", AVERAGE_MS(measured_clocks[POLY_UNIFORM_INDEX], run_numbers[POLY_UNIFORM_INDEX]));
    printf("poly_commit: %f ms\n", AVERAGE_MS(measured_clocks[POLY_COMMIT_INDEX], run_numbers[POLY_COMMIT_INDEX]));
    printf("poly_open: %f ms\n", AVERAGE_MS(measured_clocks[POLY_OPEN_INDEX], run_numbers[POLY_OPEN_INDEX]));
    printf("sign_iteration: %f ms\n", AVERAGE_MS(measured_clocks[SIGN_ITERATION_INDEX], run_numbers[SIGN_ITERATION_INDEX]));
}