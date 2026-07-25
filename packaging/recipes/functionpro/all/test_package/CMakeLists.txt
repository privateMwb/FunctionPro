cmake_minimum_required(VERSION 3.20)
project(test_package CXX)

# Retargeting: the one name to update per project — must match
# cmake_name in ../conanfile.py.
set(TESTED_PACKAGE_NAME FunctionPro)

find_package(${TESTED_PACKAGE_NAME} REQUIRED CONFIG)

add_executable(test_package src/test_package.cpp)

target_link_libraries(test_package PRIVATE ${TESTED_PACKAGE_NAME}::${TESTED_PACKAGE_NAME})
target_compile_features(test_package PRIVATE cxx_std_23)