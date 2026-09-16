#!/bin/bash -eu
# ============================================================
# .clusterfuzzlite/build.sh
#
# FunctionPro is header-only, so unlike a harness that needs to
# compile separate .cpp translation units first, this just compiles
# each fuzz target directly against the headers under include/.
#
# Add more `${SRC}/FunctionPro/fuzz/fuzz_*.cpp` harnesses here as
# they're added; each becomes its own $OUT binary.
# ============================================================

cd "${SRC}/FunctionPro"

$CXX $CXXFLAGS -std=c++20 \
  -I"${SRC}/FunctionPro/include" \
  fuzz/fuzz_function.cpp \
  $LIB_FUZZING_ENGINE \
  -o "${OUT}/fuzz_function"

$CXX $CXXFLAGS -std=c++20 \
  -I"${SRC}/FunctionPro/include" \
  fuzz/fuzz_move_only_function.cpp \
  $LIB_FUZZING_ENGINE \
  -o "${OUT}/fuzz_move_only_function"

$CXX $CXXFLAGS -std=c++20 \
  -I"${SRC}/FunctionPro/include" \
  fuzz/fuzz_function_ref.cpp \
  $LIB_FUZZING_ENGINE \
  -o "${OUT}/fuzz_function_ref"
