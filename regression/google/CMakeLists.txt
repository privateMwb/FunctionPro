# ============================================================
# Build configuration for benchmark analysis tools:
#   - regression: compares benchmark results across versions.
# ============================================================

# Fetch nlohmann_json at configure time instead of relying on a
# system package, so the build is reproducible across Termux,
# MSYS2, and CI without any OS-specific install step.
include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(nlohmann_json)

# Build the google_regressions executable.
add_executable(
    google_regression
    ${CMAKE_CURRENT_SOURCE_DIR}/regression_main.cpp
)

# Allow regression tool files to include local headers directly (e.g.
# "support/framework.h"), and reach into benchmarks/ and include/ for
# the headers this tool shares with the benchmark suite.
target_include_directories(
    google_regression
    PRIVATE
        ${CMAKE_SOURCE_DIR}/benchmarks
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# Link nlohmann_json to the google_regressions executable.
target_link_libraries(
    google_regression
    PRIVATE
        nlohmann_json::nlohmann_json
)