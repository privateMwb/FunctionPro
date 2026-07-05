# ============================================================
# CMakeLists.txt
# Build configuration for the FunctionPro library.
# ============================================================
#
# Sections:
#   1. C++ Configuration
#   2. FunctionPro Library
#   3. Tests
#   4. Benchmarks
#   5. Examples
#   6. Installation
#   7. Packaging
#
# ============================================================

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

# Collect all test source files.
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
)

# Build the test executable.
add_executable(
    tests
    ${TEST_SOURCES}
)

# Allow simple test includes.
target_include_directories(
    tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
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

# Collect all benchmark source files.
file(GLOB_RECURSE BENCH_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.cpp"
)

# Build the benchmark executable.
add_executable(
    benchmarks
    ${BENCH_SOURCES}
)

# Allow simple benchmark includes.
target_include_directories(
    benchmarks
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks
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

# Collect all example source files.
file(GLOB_RECURSE EXAMPLE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp"
)

# Build the example executable.
add_executable(
    examples
    ${EXAMPLE_SOURCES}
)

# Allow simple example includes.
target_include_directories(
    examples
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/examples
)

# Link FunctionPro to the example executable.
target_link_libraries(
    examples
    PRIVATE
    FunctionPro
)

# ============================================================================
# Installation
# ============================================================================

# Install the FunctionPro target and export it for downstream projects.
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