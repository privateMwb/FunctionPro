# ============================================================
# examples/CMakeLists.txt
# Build configuration for the examples.
# ============================================================

# Collect all example source files.
file(GLOB_RECURSE EXAMPLE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Build the example executable.
add_executable(
    examples
    ${EXAMPLE_SOURCES}
)

# Allow example files to include local headers directly (e.g. "support/framework.h").
target_include_directories(
    examples
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link the library under test into the example executable.
target_link_libraries(
    examples
    PRIVATE
        ${PROJECT_NAME}
)