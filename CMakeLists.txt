# ============================================================
# CMakeLists.txt
# Build configuration for this library.
# ============================================================
#
# Sections:
#   1. C++ Configuration
#   2. Library
#   3. Subdirectories
#   4. Installation
#   5. Packaging
#
# Retargeting this skeleton at a new library: change only the name
# passed to project() below, and rename the src/<name> and
# include/<name> directories to match. Everything else in this file
# derives from ${PROJECT_NAME} automatically.
#
# The Library section below auto-detects whether src/<name>/*.cpp
# files exist. If none are found, it builds an INTERFACE (header-only)
# target. If sources are found, it builds a STATIC library instead.
# No manual edits needed when a project grows from header-only into
# having compiled sources.
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

# Place all executables directly in the build directory, regardless of
# which subdirectory defines them (e.g. build/tests, build/compare).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ============================================================================
# Library (auto: header-only INTERFACE if no .cpp files, STATIC otherwise)
# ============================================================================

# Collect any library source files under src/<project_name>/.
# CONFIGURE_DEPENDS makes CMake re-run this glob on reconfigure, so
# adding sources to a previously header-only project is picked up
# automatically on the next cmake run — no manual editing required.
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/${PROJECT_NAME}/*.cpp"
)

if(LIB_SOURCES)
    # Sources found: build a real compiled static library.
    add_library(
        ${PROJECT_NAME}
        STATIC
        ${LIB_SOURCES}
    )

    target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    target_compile_features(
        ${PROJECT_NAME}
        PUBLIC
            cxx_std_23
    )
else()
    # No sources: header-only library, INTERFACE target.
    add_library(
        ${PROJECT_NAME}
        INTERFACE
    )

    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    target_compile_features(
        ${PROJECT_NAME}
        INTERFACE
            cxx_std_23
    )
endif()

# ============================================================================
# Subdirectories
# ============================================================================

# Consumers (e.g. vcpkg) should disable these — only the library itself
# needs to be configured/installed for downstream projects. Defaults to
# ON so local development and existing CI workflows are unaffected.
# Kept as a fixed prefix rather than ${PROJECT_NAME}-derived: downstream
# projects that reference these flags (e.g. -DBUILD_TESTS=OFF) expect a
# stable name that doesn't change when this skeleton is retargeted.
option(BUILD_TESTS "Build the test suite" ON)
option(BUILD_BENCHMARKS "Build the benchmark suite" ON)
option(BUILD_REGRESSION "Build the regression benchmark tools" ON)
option(BUILD_EXAMPLES "Build the examples suite" ON)

enable_testing()

if(BUILD_TESTS)
    add_subdirectory(tests tests_build)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks benchmarks_build)
endif()

if(BUILD_REGRESSION)
    add_subdirectory(regression regression_build)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples examples_build)
endif()

# ============================================================================
# Installation
# ============================================================================

# Install the library target and export it for downstream projects.
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
)

# Install the exported CMake targets.
install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
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
    cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

# Generate the package version file.
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install the package configuration files.
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
)