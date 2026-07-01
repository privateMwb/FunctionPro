#include "bench_helper.h"

#include <iostream>

void run_function_benchmarks();
void run_move_only_function_benchmarks();
void run_function_ref_benchmarks();

int main() {
    std::cout << "\n";

    run_function_benchmarks();
    run_move_only_function_benchmarks();
    run_function_ref_benchmarks();
    
    std::cout << "\n";
    return 0;
}