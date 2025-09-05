#!/usr/bin/env python3
"""
Conan 1.x recipe for dotenv-cpp library.
High-reliability C++ library for environment variable management.
"""

from conans import ConanFile, CMake, tools


class DotenvConan(ConanFile):
    """Conan recipe for dotenv-cpp library."""

    name = "dotenv"
    version = "2.0.0"
    license = "MIT"
    author = "dotenv-cpp team"
    url = "https://github.com/yourusername/dotenv-cpp"
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

    generators = "cmake", "cmake_find_package"
    exports_sources = "CMakeLists.txt", "src/*", "include/*", "tests/*", "benchmarks/*", "cmake/*"

    def config_options(self):
        """Configure package options based on settings."""
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        """Configure package settings."""
        if self.options.shared:
            del self.options.fPIC

        # C++20 requirement
        if self.settings.compiler.cppstd:
            tools.check_min_cppstd(self, "20")

    def build_requirements(self):
        """Declare build-time dependencies."""
        if self.options.enable_tests:
            self.build_requires("gtest/1.11.0")
        if self.options.enable_benchmarks:
            # Google Benchmark will be fetched by CMake
            pass

    def _cmake(self):
        """Configure CMake build."""
        cmake = CMake(self)

        # Configure options
        cmake.definitions["DOTENV_ENABLE_TESTS"] = self.options.enable_tests
        cmake.definitions["DOTENV_ENABLE_BENCHMARKS"] = self.options.enable_benchmarks
        cmake.definitions["DOTENV_ENABLE_SANITIZERS"] = self.options.enable_sanitizers

        # Build type specific flags
        if self.settings.build_type == "Release":
            cmake.definitions["CMAKE_BUILD_TYPE"] = "Release"
        elif self.settings.build_type == "Debug":
            cmake.definitions["CMAKE_BUILD_TYPE"] = "Debug"

        cmake.configure()
        return cmake

    def build(self):
        """Build the package."""
        cmake = self._cmake()
        cmake.build()

        # Run tests if enabled
        if self.options.enable_tests:
            cmake.test()

    def package(self):
        """Package the built artifacts."""
        cmake = self._cmake()
        cmake.install()

        # Copy license
        self.copy("LICENSE", dst="licenses")

    def package_info(self):
        """Define package information for consumers."""
        # Main library target
        self.cpp_info.libs = ["dotenv_lib"]

        # Include directories
        self.cpp_info.includedirs = ["include"]

        # Compiler features
        self.cpp_info.cppstd = "20"

        # System dependencies
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs = ["pthread"]

        # CMake target names for modern CMake
        self.cpp_info.names["cmake_find_package"] = "dotenv"
        self.cpp_info.names["cmake_find_package_multi"] = "dotenv"

        # Define component for the library
        self.cpp_info.components["dotenv_lib"].libs = ["dotenv_lib"]
        self.cpp_info.components["dotenv_lib"].includedirs = ["include"]
        self.cpp_info.components["dotenv_lib"].cppstd = "20"
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["dotenv_lib"].system_libs = ["pthread"]
