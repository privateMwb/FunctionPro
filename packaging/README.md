# Packaging: Local Verification

Notes for re-verifying the vcpkg port and Conan recipe after changes
(e.g. a new release, editing `portfile.cmake`, or editing
`packaging/recipes/functionpro/all/conanfile.py`). Scoped to what isn't obvious
from the files themselves — not a general build tutorial.

## Termux-specific quirks

Both package managers assume a "normal" Linux/CI environment.
Termux's Android-flavored profile detection breaks several of their
default assumptions. None of this applies on real CI (GitHub Actions
Ubuntu/Windows runners, a real Linux box, etc.) — it's purely a
Termux problem, and the `-s os=Linux` / `-s compiler.libcxx=libc++`
overrides below can be dropped entirely off-device.

- **`vcpkg`** — not installed on Termux; bootstrapping it needs a full
  C++ toolchain build and has known portability issues on bionic libc
  (`executable's TLS segment is underaligned`, thrown by Android's
  linker against the prebuilt binary). Skip it. A plain
  `cmake --install` reproduces what `vcpkg_cmake_configure` +
  `vcpkg_cmake_install` do under the hood — that's enough to verify
  the port without the binary.

- **`conan profile detect`** auto-detects `os=Android`, which makes
  `CMakeToolchain` demand `tools.android:ndk_path` — Termux isn't
  actually cross-compiling, so override: `-s os=Linux`.

- **`compiler.libcxx`** auto-detects as `libstdc++11`, but Termux's
  Clang actually ships `libc++`. Wrong value forces `-stdlib=libstdc++`,
  which doesn't exist here and breaks every standard header include.
  Override: `-s compiler.libcxx=libc++` (or bake it into
  `~/.conan2/profiles/default` so it doesn't need repeating per command).

- **Host/build profile mismatch** — overriding `os`/`libcxx` on the
  host profile only, while the build profile still says
  `os=Android`/`libstdc++11`, confuses Conan into partial cross-compile
  behavior (breaks include path resolution even for basic headers like
  `<cassert>`). Always override both host and build: `-s ... -s:b ...`.

- **`cmake.configure()` without the `BUILD_*` overrides silently
  configures the entire top-level project**, not just the library.
  `CMakeLists.txt` also defines `benchmarks/`, `tests/`, `regression/`, and
  `examples/` targets, and `benchmarks/` `FetchContent`-clones
  nlohmann/json at configure time. On Termux's mobile connection that
  clone can sit for a long time with zero CMake output, right after
  `-- Detecting CXX compile features - done` — looks exactly like a
  hang, not a slow clone. Always pass `-DBUILD_TESTS=OFF
  -DBUILD_BENCHMARKS=OFF -DBUILD_REGRESSION=OFF -DBUILD_EXAMPLES=OFF`
  (already baked into `conanfile.py`'s `build()` and `portfile.cmake`'s
  `vcpkg_cmake_configure`, but relevant if configuring manually).

- **The release tarball extracts to `LRUCache-<version>`, not
  `FunctionPro-<version>`** — the GitHub repo is `LRUCache`; the CMake
  project/library target inside it is `FunctionPro`. `cd` into whatever
  the tarball actually produced, not the library name. If you don't
  have a published release yet, use the local repo checkout directly
  instead (see step 1 below) — no need to cut a release just to verify
  packaging still works.

- **`CMAKE_BUILD_TYPE` matters even without a toolchain file** —
  Conan's `CMakeDeps`-generated `INTERFACE_INCLUDE_DIRECTORIES` are
  gated behind `$<$<CONFIG:Release>:...>` generator expressions. If
  `CMAKE_BUILD_TYPE` isn't set to match (e.g. left empty), the include
  directories silently evaluate to nothing — no error, just a
  "file not found" on every header. Always pass
  `-DCMAKE_BUILD_TYPE=Release` explicitly when testing.

## Verifying the vcpkg port

**1. Install to a staging prefix**, using the same disable flags
`portfile.cmake` does. Against a release tarball, `cd` into it first
(see the quirks note above on its folder name); against local source,
just start from the repo root:

```bash
cmake -B build-install \
  -DCMAKE_INSTALL_PREFIX=$HOME/staging \
  -DBUILD_TESTS=OFF \
  -DBUILD_BENCHMARKS=OFF \
  -DBUILD_REGRESSION=OFF \
  -DBUILD_EXAMPLES=OFF

cmake --build build-install
cmake --install build-install
```

**2. Confirm the CMake package files exist:**

```bash
find $HOME/staging/lib/cmake/FunctionPro -name "*.cmake"
```

Expect `FunctionProConfig.cmake`, `FunctionProConfigVersion.cmake`,
`FunctionProTargets.cmake` (plus a per-config
`FunctionProTargets-<config>.cmake`) under `lib/cmake/FunctionPro/`.

**3. Build a minimal consumer project**, in a separate directory so
it can't accidentally pick up anything from the library's own build
tree:

```bash
mkdir -p ~/staging-consumer && cd ~/staging-consumer

cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.20)
project(test CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(FunctionPro CONFIG REQUIRED)
add_executable(test main.cpp)
target_link_libraries(test PRIVATE FunctionPro::FunctionPro)
EOF

cat > main.cpp << 'EOF'
#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>
#include <FunctionPro/FunctionRef.h>
int main() {
    rain::Function<int(int, int)> f;
    return 0;
}
EOF

cmake -B build -DCMAKE_PREFIX_PATH=$HOME/staging
cmake --build build
./build/test
echo "Exit code: $?"
```

A clean run prints `Exit code: 0` and nothing else — the consumer
just default-constructs a `FunctionPro::LRUCache` and returns, so success
already confirms the header parsed, the library linked, and the
symbols resolved.

If you're re-testing against multiple libraries that share this
skeleton, use a distinct `$HOME/staging-<project>` prefix per
project — a shared staging directory accumulates every project's
package files side by side, and a later `find_package()` call could
silently resolve the wrong one.

## Verifying the Conan recipe

**1. Build and package the recipe.** The `os`/`libcxx` overrides are
the Termux-specific ones from the quirks section above — drop them
entirely on real CI or a normal Linux machine:

```bash
conan create packaging/recipes/functionpro/all --version 1.0.0 \
  -s compiler.cppstd=23 -s os=Linux -s:b os=Linux \
  -s compiler.libcxx=libc++ -s:b compiler.libcxx=libc++
```

Watch for: `validate()` passing (the C++23 floor check), `LICENSE`
landing in the `package()` step's output, and — since this builds a
real static library, not a header-only one — a `package_id` computed
per settings combination rather than one shared id.

This step also automatically builds and runs
`packaging/recipes/functionpro/all/test_package/` against the freshly-built
package — a `FunctionPro linked and constructed successfully.` line in
the output means that already passed, before you even get to step 2.

**2. Build a second, independent consumer project** — this is a
different check than `test_package`: it confirms the package is
consumable the way a real downstream project would consume it
(`conanfile.txt` + `CMakeDeps`/`CMakeToolchain`), not just that the
recipe's own bundled test passes.

```bash
mkdir -p ~/conan-consumer && cd ~/conan-consumer

cat > conanfile.txt << 'EOF'
[requires]
functionpro/1.0.0

[generators]
CMakeDeps
CMakeToolchain
EOF

cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.20)
project(test CXX)
find_package(FunctionPro CONFIG REQUIRED)
add_executable(test main.cpp)
target_link_libraries(test PRIVATE FunctionPro::FunctionPro)
EOF

cat > main.cpp << 'EOF'
#include <FunctionPro/Function.h>
#include <FunctionPro/MoveOnlyFunction.h>
#include <FunctionPro/FunctionRef.h>
int main() {
    rain::Function<int(int, int)> f;
    return 0;
}
EOF

conan install . -s compiler.cppstd=23 -s os=Linux -s:b os=Linux \
  -s compiler.libcxx=libc++ -s:b compiler.libcxx=libc++ --build=missing

cmake -B build -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/test
echo "Exit code: $?"
```

Same expectation as the vcpkg run: `Exit code: 0`, no other output.
