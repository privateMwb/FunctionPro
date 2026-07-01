cmake_minimum_required(VERSION 3.20)

project(
    FunctionPro
    VERSION 1.0.0
    LANGUAGES CXX
)


# ============================================================================
# C++ Configuration
# ============================================================================

# Configure the C++ language standard.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# ============================================================================
# FunctionPro (Header-only Library)
# ============================================================================

# Define the header-only FunctionPro library.
add_library(FunctionPro INTERFACE)

# Expose the public include directory to consumers.
target_include_directories(
    FunctionPro
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Require C++23 for all targets that consume FunctionPro.
target_compile_features(
    FunctionPro
    INTERFACE
    cxx_std_23
)


# ============================================================================
# Tests
# ============================================================================

# Build the test executable.
add_executable(
    tests
    tests/test_all.cpp
    tests/test_function.cpp
    tests/test_move_only_function.cpp
    tests/test_function_ref.cpp
)

# Link FunctionPro to the test executable.
target_link_libraries(
    tests
    PRIVATE
    FunctionPro
)


# ============================================================================
# Benchmarks
# ============================================================================

# Build the benchmark executable.
add_executable(
    benchmarks
    benchmarks/bench_all.cpp
    benchmarks/bench_function.cpp
    benchmarks/bench_move_only_function.cpp
    benchmarks/bench_function_ref.cpp
)

# Link FunctionPro to the benchmark executable.
target_link_libraries(
    benchmarks
    PRIVATE
    FunctionPro
)


# ============================================================================
# Examples
# ============================================================================

# Build the Function usage example.
add_executable(
    example_function
    examples/example_function.cpp
)

# Link FunctionPro to the Function usage example.
target_link_libraries(
    example_function
    PRIVATE
    FunctionPro
)

# Build the MoveOnlyFunction usage example.
add_executable(
    example_move_only_function
    examples/example_move_only_function.cpp
)

# Link FunctionPro to the MoveOnlyFunction usage example.
target_link_libraries(
    example_move_only_function
    PRIVATE
    FunctionPro
)

# Build the FunctionRef usage example.
add_executable(
    example_function_ref
    examples/example_function_ref.cpp
)

# Link FunctionPro to the FunctionRef usage example.
target_link_libraries(
    example_function_ref
    PRIVATE
    FunctionPro
)


# ============================================================================
# Installation
# ============================================================================

# Install the FunctionPro target.
install(
    TARGETS FunctionPro
    EXPORT FunctionProTargets
)

# Install the exported CMake targets.
install(
    EXPORT FunctionProTargets
    FILE FunctionProTargets.cmake
    NAMESPACE FunctionPro::
    DESTINATION lib/cmake/FunctionPro
)

# Install the public header files.
install(
    DIRECTORY include/
    DESTINATION include
)


# ============================================================================
# Packaging
# ============================================================================

include(CMakePackageConfigHelpers)

# Generate the package configuration file.
configure_package_config_file(
    cmake/FunctionProConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/FunctionProConfig.cmake
    INSTALL_DESTINATION lib/cmake/FunctionPro
)

# Generate the package version file.
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/FunctionProConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install the package configuration files.
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/FunctionProConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/FunctionProConfigVersion.cmake
    DESTINATION lib/cmake/FunctionPro
)

