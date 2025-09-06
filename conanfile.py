#!/usr/bin/env python3
"""
Conan 2.0 recipe for dotenv-cpp library.
High-reliability C++ library for environment variable management.
"""

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy
from conan.errors import ConanInvalidConfiguration


class DotenvConan(ConanFile):
    """Conan recipe for dotenv-cpp library."""

    name = "dotenv"
    version = "2.0.0"
    license = "MIT"
    author = "dotenv-cpp team"
    url = "https://github.com/Fabio3rs/dotenv-cpp"
    description = "High-performance, thread-safe C++20 library for .env file parsing"
    topics = ("cpp", "environment", "config", "dotenv", "high-performance")

    # Package settings
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "enable_tests": [True, False],
        "enable_benchmarks": [True, False],
        "enable_sanitizers": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "enable_tests": False,
        "enable_benchmarks": False,
        "enable_sanitizers": False
    }

    exports_sources = "CMakeLists.txt", "src/*", "include/*", "tests/*", "benchmarks/*", "cmake/*"

    def config_options(self):
        """Configure package options based on settings."""
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        """Configure package settings."""
        if self.options.shared:
            del self.options.fPIC

    def validate(self):
        """Validate package configuration."""
        # Require C++20
        from conan.tools.build import check_min_cppstd
        check_min_cppstd(self, "20")

    def build_requirements(self):
        """Declare build-time dependencies."""
        if self.options.enable_tests:
            self.test_requires("gtest/1.11.0")
        if self.options.enable_benchmarks:
            # Google Benchmark will be fetched by CMake
            pass

    def layout(self):
        """Define layout for the package."""
        cmake_layout(self)

    def generate(self):
        """Generate files needed for build."""
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)

        # Configure options
        tc.variables["DOTENV_ENABLE_TESTS"] = self.options.enable_tests
        tc.variables["DOTENV_ENABLE_BENCHMARKS"] = self.options.enable_benchmarks
        tc.variables["DOTENV_ENABLE_SANITIZERS"] = self.options.enable_sanitizers

        tc.generate()

    def build(self):
        """Build the package."""
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        # Run tests if enabled
        if self.options.enable_tests:
            cmake.test()

    def package(self):
        """Package the built artifacts."""
        cmake = CMake(self)
        cmake.install()

        # Copy license
        copy(self, "LICENSE", src=self.source_folder, dst=self.package_folder / "licenses")

    def package_info(self):
        """Define package information for consumers."""
        # Main library component
        self.cpp_info.components["dotenv_lib"].libs = ["dotenv_lib"]
        self.cpp_info.components["dotenv_lib"].includedirs = ["include"]
        self.cpp_info.components["dotenv_lib"].cppstd = "20"

        # System dependencies
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["dotenv_lib"].system_libs = ["pthread"]

        # Default component for backward compatibility
        self.cpp_info.components["dotenv_lib"].set_property("cmake_target_name", "dotenv::dotenv_lib")

        # Legacy names for older consumers
        self.cpp_info.set_property("cmake_file_name", "dotenv")
        self.cpp_info.set_property("cmake_target_name", "dotenv::dotenv")
