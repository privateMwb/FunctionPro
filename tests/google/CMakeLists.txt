# ============================================================
# Build configuration for the GoogleTest-based test suite.
# ============================================================

# Fetch GoogleTest.
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Match the runtime library MSVC uses, so linking doesn't mismatch.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Collect all test source files.
file(GLOB_RECURSE GTEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Build the test executable.
add_executable(
    google_tests
    ${GTEST_SOURCES}
)

# Link the library under test and GoogleTest into the test executable.
target_link_libraries(
    google_tests
    PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
)

# Register each TEST() as its own entry with ctest.
include(GoogleTest)
gtest_discover_tests(google_tests)