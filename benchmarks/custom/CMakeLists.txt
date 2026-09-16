# ============================================================
# benchmarks/CMakeLists.txt
# Build configuration for the project benchmark suite.
# ============================================================

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Collect all benchmark suite source files.
file(GLOB_RECURSE BENCH_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/suite/*.cpp"
)

# Build the benchmark executable.
add_executable(
    benchmarks
    ${BENCH_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/bench_main.cpp
)

# Allow simple benchmark includes.
target_include_directories(
    benchmarks
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# Link the library under test into the benchmark executable.
target_link_libraries(
    benchmarks
    PRIVATE
        ${PROJECT_NAME}
)