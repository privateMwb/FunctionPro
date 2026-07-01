#include "test_helper.h"

void run_function_tests();
void run_move_only_function_tests();
void run_function_ref_tests();

int main() {
std::cout << "\n";

run_function_tests();
run_move_only_function_tests();
run_function_ref_tests();

stats();
std::cout << "\n";
return 0;
}
