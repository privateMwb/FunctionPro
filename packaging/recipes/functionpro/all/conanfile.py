from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, get
import os


class Conan(ConanFile):
    # ── Retargeting this recipe for a new library ───────────────────
    # Edit these fields (and the class name above) — everything below
    # derives from them. Version is handled by a separate script, not
    # edited here.
    name = "functionpro"
    cmake_name = "FunctionPro"  # matches project()'s name in the top-level CMakeLists.txt
    version = "1.0.0"

    url = "https://github.com/privateMwb/FunctionPro"
    description = "High-performance, header-only type-erased callable wrappers with small-buffer optimization — Function, MoveOnlyFunction, and FunctionRef."
    topics = (
        "callable",
        "function-wrapper",
        "type-erasure",
        "sbo",
        "cpp",
    )
    # ──────────────────────────────────────────────────────────────

    # header-library, not "library": there's no compiled src/FunctionPro/*.cpp
    # today, so CMakeLists.txt's auto-detect builds an INTERFACE target.
    # NOTE: if this library ever grows compiled sources, this recipe needs
    # to switch back to "library" and package_id()/cpp_info.libs need to
    # become conditional again, mirroring CMakeLists.txt's own auto-detect.
    package_type = "header-library"

    license = "MIT"
    author = "privateMwb"

    settings = "os", "compiler", "build_type", "arch"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }

    default_options = {
        "shared": False,
        "fPIC": True,
    }

    exports_sources = (
        "CMakeLists.txt",
        "cmake/*",
        "include/*",
        "src/*",
    )

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def package_id(self):
        # Header-only: no compiled ABI, so one package serves every
        # compiler/arch/build_type combination.
        self.info.clear()

    def validate(self):
        check_min_cppstd(self, 23)

    def source(self):
        get(
            self,
            **self.conan_data["sources"][self.version],
            strip_root=True,
        )

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(
            variables={
                "BUILD_TESTS": "OFF",
                "BUILD_BENCHMARKS": "OFF",
                "BUILD_REGRESSION": "OFF",
                "BUILD_EXAMPLES": "OFF",
            }
        )
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

        copy(
            self,
            "LICENSE",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", self.cmake_name)
        self.cpp_info.set_property("cmake_target_name", f"{self.cmake_name}::{self.cmake_name}")
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []