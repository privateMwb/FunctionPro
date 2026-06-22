#pragma once

#include <iostream>
#include <cstdlib>

#define RUN(name) do {                           \
    name();                                      \
    std::cout << "[PASS]: " << #name << "\n";    \
} while (0)


#define CHK(expr) do {                           \
    if(!(expr)) {                                  \
	    std::cout << "[FAIL]: " << #expr         \
	              << " (" << __FILE__            \
	              << ":" << __LINE__             \
	              << ")\n";                      \
    }                                            \
} while (0)

// Shared Callable
inline int   free_add(int x, int y)  { return x + y; }
inline void  free_side(int& x)       { x++; }

