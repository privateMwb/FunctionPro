# ============================================================
# tests/CMakeLists.txt
# Build configuration for the test suite.
# ============================================================

# Collect all test source files.
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Build the test executable.
add_executable(
    tests
    ${TEST_SOURCES}
)

# Allow test files to include local headers directly (e.g. "support/framework.h").
target_include_directories(
    tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link the library under test into the test executable.
target_link_libraries(
    tests
    PRIVATE
        ${PROJECT_NAME}
)

# Register the test executable with ctest.
add_test(
    NAME ${PROJECT_NAME}Tests
    COMMAND tests
)