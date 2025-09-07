# dotenv-cpp

A high-performance, thread-safe C++20 library for managing `.env` files. The library supports both C++ and C interfaces, offering modern APIs with comprehensive error handling and security features.

🎉 **Release Candidate v2.0.0**
✅ Full test coverage, security audited, and performance optimized
✅ Modern C++20 with backward compatibility to C
✅ Thread-safe operations with comprehensive benchmarks

## Features

- 🚀 **High Performance**: Load operations at 126k/s for small files, 13k/s for medium files
- 🔒 **Thread Safe**: Concurrent reads at 5.17M ops/s, validated with comprehensive benchmarks
- 🛡️ **Memory Safe**: RAII-based design with zero raw pointers, full sanitizer testing
- 🎯 **Modern C++20**: Type-safe templates, `std::string_view`, structured bindings
- 📦 **Easy Integration**: CMake package config, Conan support, header-only option
- 🔧 **Flexible APIs**: No-throw policies, default values, error handling options
- ⚡ **Optimized Parsing**: Handles comments, quotes, escape sequences efficiently
- 🌐 **Cross Platform**: Windows, Linux, macOS with consistent behavior
- 📋 **C Compatibility**: Legacy C interface for broader integration
- 🧪 **Fully Tested**: 13/13 tests passing

## Quick Start

### Using Conan

#### Option 1: Local Package (Recommended for now)

Since the package is not yet published to Conan Center, you can build and use it locally:

1. **Create local package:**
   ```bash
   # Clone the repository
   git clone https://github.com/Fabio3rs/dotenv-cpp.git
   cd dotenv-cpp

   # Create local Conan package
   conan create . --build=missing
   ```

2. **Use in your project's `conanfile.txt`:**
   ```plaintext
   [requires]
   dotenv/2.0.0

   [generators]
   CMakeDeps
   CMakeToolchain
   ```

3. **Install and link in your project:**
   ```bash
   # In your project directory
   conan install . --output-folder=build --build=missing
   ```

4. **CMakeLists.txt integration:**
   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(my_project)

   find_package(dotenv REQUIRED)

   add_executable(my_project main.cpp)
   target_link_libraries(my_project PRIVATE dotenv::dotenv_lib)
   target_compile_features(my_project PRIVATE cxx_std_20)
   ```

#### Option 2: Remote Repository (Advanced)

For team/organization use, you can add the package to a custom Conan remote:

1. **Create and upload to remote:**
   ```bash
   # Add your remote (replace with your Artifactory/remote URL)
   conan remote add my-remote https://my-artifactory.com/artifactory/api/conan/my-repo

   # Create and upload package
   conan create . --build=missing
   conan upload dotenv/2.0.0 -r my-remote --all
   ```

2. **Use from remote:**
   ```bash
   # Team members can install from remote
   conan install dotenv/2.0.0@ --build=missing -r my-remote
   ```

#### Option 3: From Conan Center (Future)

***Note:*** *This method will be available once the package is published to Conan Center.*

   ```plaintext
   [requires]
   dotenv/2.0.0
   ```

### Manual Integration

1. Clone the repository:
   ```bash
   git clone https://github.com/Fabio3rs/dotenv-cpp.git
   ```

2. Include the `include` directory in your project's include paths.

3. Add the `src` files to your build system.

---

## Conan Package Options

When using Conan, you can customize the build with these options:

```bash
# Basic usage
conan create . --build=missing

# Enable tests and benchmarks
conan create . -o enable_tests=True -o enable_benchmarks=True --build=missing

# Build as shared library
conan create . -o shared=True --build=missing

# Debug build with sanitizers
conan create . -s build_type=Debug -o enable_sanitizers=True --build=missing
```

### Available Options:
- `shared`: Build as shared library (default: False)
- `fPIC`: Position independent code (default: True)
- `enable_tests`: Build and run tests (default: False)
- `enable_benchmarks`: Build benchmarks (default: False)
- `enable_sanitizers`: Enable AddressSanitizer and UBSan (default: False)

### Complete Example

For a complete working example using Conan, see [`examples/conan_usage/`](examples/conan_usage/).

### Additional Documentation

- 📋 **[Platform Compatibility Matrix](docs/COMPATIBILITY.md)** - Detailed compiler and platform support
- 🔒 **[Security Guide](docs/SECURITY.md)** - Comprehensive security considerations and best practices

---

## Usage

### C++ API

Include `dotenv.hpp` and use the `dotenv` namespace:

```cpp
#include "dotenv.hpp"
#include <iostream>

int main() {
    // Modern C++23 usage with std::expected (recommended)
    auto result = dotenv::load(".env");
    if (result) {
        std::cout << "Loaded " << *result << " variables\n";
    } else {
        std::cout << "Failed to load .env: " << static_cast<int>(result.error()) << "\n";
        return 1;
    }

    // Type-safe variable access
    auto port = dotenv::get<int>("PORT", 3000);
    auto debug = dotenv::get<bool>("DEBUG", false);
    std::string_view app_name = dotenv::get("APP_NAME", "MyApp");

    std::cout << "App: " << app_name << " on port " << port
              << (debug ? " (debug mode)" : "") << std::endl;

    // Safe internal-only loading (won't affect system environment)
    auto config_result = dotenv::load("config.env", {
        .apply_to_process = dotenv::process_env_apply::no
    });

    // Save current state
    dotenv::set("LAST_RUN", "2024-09-06");
    dotenv::save_to_file(".env");

    return 0;
}
```

**Legacy Usage (still supported but deprecated):**
```cpp
#include "dotenv.hpp"
#include <iostream>

int main() {
    // Legacy interface (deprecated but still works)
    int loaded = dotenv::load(".env", 1, true);
    if (loaded < 0) {
        std::cerr << "Failed to load .env\n";
        return 1;
    }

    std::string_view value = dotenv::get("MY_KEY", "default_value");
    std::cout << "MY_KEY: " << value << std::endl;

    return 0;
}
```

### C API

Include `dotenv.h` and use the C functions:

```c
#include "dotenv.h"
#include <stdio.h>

int main() {
    // Load .env file and apply variables to system environment
    dotenv_load(".env", 1, 1);

    // Load .env file but keep variables internal only (don't modify system env)
    // dotenv_load(".env", 1, 0);

    const char *value = dotenv_get("MY_KEY", "default_value");
    printf("MY_KEY: %s\n", value);

    return 0;
}
```

---

## API Reference

### C++ API (`dotenv.hpp`)

#### Basic Loading Functions

- **`int dotenv::load(std::string_view path = ".env", int replace = 1, bool apply_system_env = true)`**
  **[DEPRECATED]** Legacy interface. Use modern `load(path, load_options{})` instead.
  Loads the environment variables from the specified `.env` file.
  - `path`: Path to the `.env` file (default: `.env`).
  - `replace`: Replace existing variables (default: `1`).
  - `apply_system_env`: Whether to apply variables to system environment via setenv() (default: `true`). When `false`, variables are only stored internally.
  - Returns: Number of variables loaded, or negative error code.

- **`int dotenv::load_traditional(std::string_view path, int replace, bool apply_system_env)`**
  **[DEPRECATED]** Legacy interface. Use modern `load_traditional(path, load_options{})` instead.
  Forces traditional implementation (no SIMD optimization).

#### Modern Loading Functions (Recommended)

- **`std::expected<int, dotenv::dotenv_error> dotenv::load(std::string_view path, const load_options& options = {})`**
  **[C++23 RECOMMENDED]** Modern type-safe interface with std::expected.
  - `path`: Path to the `.env` file.
  - `options`: Type-safe configuration structure.
  - Returns: Expected containing variable count, or unexpected containing error.
  - Example:
    ```cpp
    // Modern usage with type-safe options
    auto result = dotenv::load(".env", {
        .overwrite_policy = dotenv::overwrite::replace,
        .apply_to_process = dotenv::process_env_apply::no  // Keep variables internal only
    });

    if (result) {
        std::cout << "Loaded " << *result << " variables\n";
    } else {
        std::cout << "Error: " << static_cast<int>(result.error()) << "\n";
    }
    ```

- **`std::expected<int, dotenv::dotenv_error> dotenv::load_traditional(std::string_view path, const load_options& options = {})`**
  Modern interface forcing traditional implementation.

- **`std::expected<int, dotenv::dotenv_error> dotenv::load_simd(std::string_view path, const load_options& options = {})`** (if SIMD enabled)
  Modern interface forcing SIMD-optimized implementation.

#### Load Options Configuration

The `load_options` struct provides type-safe configuration:

```cpp
namespace dotenv {
    enum class overwrite {
        preserve,  // Don't overwrite existing variables
        replace    // Replace existing variables (default)
    };

    enum class process_env_apply {
        no,   // Keep variables internal only (safe)
        yes   // Apply to system environment (default)
    };

    enum class parse_backend {
        auto_detect,  // Automatically choose best backend (default)
        traditional,  // Force traditional parser
        simd         // Force SIMD parser (if available)
    };

    struct load_options {
        overwrite overwrite_policy = overwrite::replace;
        process_env_apply apply_to_process = process_env_apply::yes;
        parse_backend backend = parse_backend::auto_detect;
    };
}
```

**Usage Examples:**
```cpp
// Safe loading (won't affect system environment)
auto result = dotenv::load(".env", {
    .apply_to_process = dotenv::process_env_apply::no
});

// Conservative loading (won't overwrite existing variables)
auto result = dotenv::load(".env", {
    .overwrite_policy = dotenv::overwrite::preserve,
    .apply_to_process = dotenv::process_env_apply::yes
});

// Force specific backend
auto result = dotenv::load("large.env", {
    .backend = dotenv::parse_backend::simd
});
```

#### Enhanced Error Handling (C++20/C++23)

**C++20 - Structured Bindings with std::pair:**
- **`std::pair<dotenv::dotenv_error, int> dotenv::load_legacy(path, options)`**
  Loads variables with detailed error information using legacy pair interface.
  - Returns: `{status_code, variables_loaded}` where status is from `dotenv::dotenv_error` enum.
  - Example:
    ```cpp
    auto [status, count] = dotenv::load_legacy(".env");
    if (status == dotenv::dotenv_error::success) {
        std::cout << "Loaded " << count << " variables\n";
    } else {
        std::cout << "Error: " << static_cast<int>(status) << "\n";
    }
    ```

- **`std::pair<dotenv::dotenv_error, int> dotenv::load_traditional_legacy(...)`**
  Traditional implementation with detailed status.

- **`std::pair<dotenv::dotenv_error, int> dotenv::load_simd_legacy(...)`** (if SIMD enabled)
  SIMD implementation with detailed status.

**C++23 - Modern std::expected (Recommended):**
- **`std::expected<int, dotenv::dotenv_error> dotenv::load(path, options)`**
  Modern error handling using std::expected with type-safe load_options.
  - Returns: Expected containing variable count, or unexpected containing error code.
  - Example:
    ```cpp
    auto result = dotenv::load(".env", {.replace_existing = true, .apply_to_process = false});
    if (result) {
        std::cout << "Loaded " << *result << " variables\n";
    } else {
        std::cout << "Error: " << static_cast<int>(result.error()) << "\n";
    }

    // Monadic operations
    auto config = dotenv::load(".env", {})
        .and_then([](int count) { return process_config(count); })
        .or_else([](auto error) { return use_defaults(error); });
    ```

- **`std::expected<int, dotenv::dotenv_error> dotenv::load_traditional(path, options)`**
  Traditional implementation with std::expected.

- **`std::expected<int, dotenv::dotenv_error> dotenv::load_simd(path, options)`** (if SIMD enabled)
  SIMD implementation with std::expected.

#### Error Codes (`dotenv_types.h`)

```cpp
namespace dotenv {
enum class dotenv_error {
    success = 0,                    // Operation successful
    file_not_found = -1,           // .env file not found
    permission_denied = -2,        // Permission denied
    invalid_format = -3,           // Invalid file format
    out_of_memory = -4,            // Insufficient memory
    invalid_argument = -5,         // Invalid argument
    buffer_too_small = -6,         // Buffer too small
    key_not_found = -7             // Key not found in environment
};
}
```

#### Variable Access Functions

- **`std::string_view dotenv::get(std::string_view key, std::string_view default_value = "")`**
  Retrieves the value of the given key or a default value if the key doesn't exist.

- **`template<typename T> T dotenv::get(std::string_view key, T default_value = {})`**
  Type-safe template version for numeric types (int, float, double, bool).
  - Automatically parses string values to the requested type.
  - Returns default_value if key not found or parsing fails.
  - Example:
    ```cpp
    auto port = dotenv::get<int>("PORT", 3000);
    auto timeout = dotenv::get<double>("TIMEOUT", 5.0);
    auto debug = dotenv::get<bool>("DEBUG", false);
    ```

- **`std::optional<T> dotenv::try_value(std::string_view key)`**
  Returns optional containing parsed value, or std::nullopt if not found/invalid.
  - Example:
    ```cpp
    if (auto port = dotenv::try_value<int>("PORT")) {
        std::cout << "Port: " << *port << "\n";
    } else {
        std::cout << "PORT not configured\n";
    }
    ```

- **`std::expected<std::string, dotenv::dotenv_error> dotenv::value_expected(std::string_view key)`** (C++23)
  Modern error handling for required variables.
  - Returns expected containing value, or unexpected containing error.
  - Example:
    ```cpp
    auto db_url = dotenv::value_expected("DATABASE_URL");
    if (db_url) {
        connect_to_database(*db_url);
    } else {
        std::cout << "DATABASE_URL required but not found\n";
    }
    ```

- **`template<typename T> std::expected<T, dotenv::dotenv_error> dotenv::value_expected(std::string_view key)`** (C++23)
  Type-safe version with parsing and error handling.

- **`bool dotenv::has(std::string_view key)`**
  Checks if a key exists.

- **`void dotenv::set(std::string_view key, std::string_view value, bool replace = true)`**
  Sets a key-value pair. Replaces the value if the key already exists and `replace` is true.

- **`void dotenv::save_to_file(std::string_view path)`**
  Saves the current environment variables to the specified `.env` file.

### C API (`dotenv.h`)

- **`int dotenv_load(const char *path, int replace, int apply_system_env)`**
  Loads environment variables from a `.env` file.
  - `path`: Path to the `.env` file.
  - `replace`: Replace existing variables.
  - `apply_system_env`: Whether to apply variables to system environment (1=apply, 0=internal only).

- **`const char *dotenv_get(const char *key, const char *default_value)`**
  Retrieves the value of the given key or a default value if the key doesn't exist.

---

## Platform Support

### Tested Compiler Matrix

| Compiler | Version | Windows | Linux | macOS | Sanitizers | SIMD (AVX2) |
|----------|---------|---------|-------|-------|------------|-------------|
| **GCC** | 10+ | ❌ | ✅ | ❌ | ASan, UBSan, LSan | ✅ |
| **GCC** | 11+ | ❌ | ✅ | ❌ | ASan, UBSan, LSan, TSan¹ | ✅ |
| **Clang** | 12+ | ✅² | ✅ | ✅ | ASan, UBSan, LSan, MSan³ | ✅ |
| **Clang** | 14+ | ✅² | ✅ | ✅ | Full suite⁴ | ✅ |
| **MSVC** | 2019 (v142) | ✅ | ❌ | ❌ | ASan⁵ | ✅ |
| **MSVC** | 2022 (v143) | ✅ | ❌ | ❌ | ASan⁵ | ✅ |

**Notes:**
1. TSan may have false positives on some threading patterns
2. Requires Clang-cl or LLVM toolchain on Windows
3. MSan requires special runtime and may need custom builds
4. Includes implicit-conversion and unsigned-integer-overflow
5. MSVC ASan has limited feature set compared to Clang/GCC

### Operating System Specifics

#### **Linux (Primary Target)**
- **Kernel**: 4.0+ (for modern syscalls and security features)
- **Distributions**: Ubuntu 20.04+, RHEL 8+, Debian 11+, Arch Linux
- **Features**: Full feature set, all sanitizers, optimal performance
- **Thread Safety**: Native pthread support

#### **Windows 10/11**
- **API Level**: Windows 10 1903+ (for Unicode environment handling)
- **Features**: Full C++20 support, limited sanitizer support
- **Unicode**: UTF-16 ↔ UTF-8 conversion for environment variables
- **Thread Safety**: Windows CRT thread-safe functions

#### **macOS**
- **Version**: 10.15+ (Catalina, for C++20 library support)
- **Xcode**: 12+ (for Clang 12+ with full C++20)
- **Features**: Full feature set except some Clang-specific sanitizers
- **Thread Safety**: Native pthread support

---

## Security Considerations

### `apply_system_env` Parameter Semantics

The `apply_system_env` parameter controls whether parsed variables are applied to the system environment via platform-specific APIs:

#### **Behavior by Platform:**

| Platform | `apply_system_env=true` | `apply_system_env=false` |
|----------|------------------------|--------------------------|
| **Linux/macOS** | Uses `setenv()` - affects child processes | Internal storage only |
| **Windows** | Uses `_wputenv_s()` - affects child processes | Internal storage only |

#### **Security Implications:**

⚠️ **CRITICAL**: When `apply_system_env=true`, parsed variables **WILL OVERRIDE** existing system environment variables if `replace=1`.

```cpp
// ⚠️ SECURITY RISK: Can override sensitive system variables
dotenv::load(".env", 1, true);  // replace=1, apply_system_env=true

// ✅ SAFE: Only internal storage, won't affect system
dotenv::load(".env", 1, false); // apply_system_env=false

// ✅ SAFE: Won't override existing variables
dotenv::load(".env", 0, true);  // replace=0, won't override existing
```

#### **Recommended Security Practices:**

1. **Production Environments:**
   ```cpp
   // Load without affecting system environment
   dotenv::load(".env", 1, false);

   // Manually validate and apply critical variables
   if (auto db_url = dotenv::get("DATABASE_URL"); !db_url.empty()) {
       if (validate_database_url(db_url)) {
           setenv("DATABASE_URL", db_url.data(), 1);
       }
   }
   ```

2. **Development Environments:**
   ```cpp
   // Safe for development, won't override existing
   dotenv::load(".env", 0, true);
   ```

3. **Sensitive Variables Protection:**
   ```cpp
   // Protect critical system variables
   const std::unordered_set<std::string> protected_vars = {
       "PATH", "HOME", "USER", "SHELL", "PWD",
       "SSH_AUTH_SOCK", "SUDO_USER", "TERM"
   };

   // Custom loading with protection
   dotenv::load(".env", 1, false);  // Load internally first

   for (const auto& [key, value] : get_all_loaded_vars()) {
       if (protected_vars.find(key) == protected_vars.end()) {
           setenv(key.c_str(), value.c_str(), 1);  // Only apply non-protected
       }
   }
   ```

#### **Thread Safety Notes:**

- **Internal storage** (`apply_system_env=false`): Thread-safe with mutex protection
- **System environment** (`apply_system_env=true`): Platform-dependent thread safety
  - Linux/macOS: `setenv()` is generally thread-safe
  - Windows: `_wputenv_s()` is thread-safe
  - **Recommendation**: Avoid concurrent writes to system environment

---

## Building from Source

### Requirements

- **C++20-compliant compiler** (see [Platform Support](#platform-support))
- **CMake 3.15** or later
- **Operating System**: Windows 10+, Linux (kernel 4.0+), macOS 10.15+

### Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/Fabio3rs/dotenv-cpp.git
   cd dotenv-cpp
   ```

2. Configure and build:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build .
   ```

3. Run tests (if enabled):
   ```bash
   ctest
   ```

### Advanced Build Options

#### **Sanitizer Builds (Recommended for Development)**

```bash
# Full sanitizer suite (Linux/macOS with Clang 14+)
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DDOTENV_ENABLE_SANITIZERS=ON \
         -DDOTENV_ENABLE_TESTS=ON

# Specific sanitizers (if full suite causes issues)
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCMAKE_CXX_FLAGS="-fsanitize=address -fsanitize=undefined -g"

# Windows MSVC with AddressSanitizer
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DDOTENV_ENABLE_SANITIZERS=ON \
         -T ClangCL  # Use Clang-CL for better sanitizer support
```

#### **Performance Builds**

```bash
# Maximum performance with SIMD
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DDOTENV_ENABLE_SIMD=ON

# Benchmarking build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DDOTENV_ENABLE_SIMD=ON \
         -DDOTENV_ENABLE_BENCHMARKS=ON
```

#### **CI/Production Builds**

```bash
# Strict warnings-as-errors build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DDOTENV_ENABLE_WERROR=ON \
         -DDOTENV_ENABLE_TESTS=ON
```

### Compiler-Specific Notes

#### **GCC Considerations**
```bash
# Minimum version check
g++ --version  # Requires 10+

# Recommended flags for development
export CXXFLAGS="-Wall -Wextra -Wconversion -Wsign-conversion -g"
```

#### **Clang Considerations**
```bash
# Clang 12+ for full C++20 support
clang++ --version

# Enable all available sanitizers (Clang 14+)
export CXXFLAGS="-fsanitize=address,undefined,implicit-conversion,unsigned-integer-overflow"
```

#### **MSVC Considerations**
```bash
# Visual Studio 2019 16.11+ or 2022
# Use vcpkg for dependencies
vcpkg install gtest:x64-windows benchmark:x64-windows

# CMake with vcpkg toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

## Troubleshooting

### Common Build Issues

#### **C++20 Support**
```bash
# Error: "C++20 features not available"
# Solution: Update compiler or use older standard
cmake .. -DCMAKE_CXX_STANDARD=17  # Fallback to C++17 if needed
```

#### **AVX2 Not Available**
```bash
# Warning: "AVX2 not supported by compiler"
# Solution: Disable SIMD optimizations
cmake .. -DDOTENV_ENABLE_SIMD=OFF
```

#### **Sanitizer Conflicts**
```bash
# Error: Multiple sanitizers conflict
# Solution: Use compatible combinations
cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=address"  # AddressSanitizer only
```

#### **Windows Unicode Issues**
```cpp
// Problem: Environment variables with non-ASCII characters
// Solution: Ensure UTF-8 encoding in .env files
// The library automatically handles UTF-16 ↔ UTF-8 conversion
```

### Performance Troubleshooting

#### **Slow Load Times**
- **Check file size**: SIMD auto-detection threshold is 50KB
- **Verify SIMD**: Use `load_simd()` for large files if auto-detection fails
- **Disable system env**: Use `apply_system_env=false` for pure parsing benchmarks

#### **Memory Usage**
- **Large files**: Consider streaming parser for files >100MB
- **Many variables**: Monitor `ValueStruct` memory usage
- **Thread contention**: Reduce concurrent writes to environment

---

## Contributing

Contributions are welcome! Please submit issues and pull requests to improve the library.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
