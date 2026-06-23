#include "benchmark_helper.h"

#include <iostream>

void run_function_benchmarks();
void run_move_only_function_benchmarks();
void run_function_ref_benchmarks();

int main() {
    bench_header("Function");
    run_function_benchmarks();

    bench_header("Move Only Function");
    run_move_only_function_benchmarks();

    bench_header("Function Ref");
    run_function_ref_benchmarks();
    
    return 0;
}