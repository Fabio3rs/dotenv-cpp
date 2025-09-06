# Conan Usage Example

This example demonstrates how to use dotenv-cpp in your project via Conan package manager.

## Prerequisites

- Conan 2.0+
- CMake 3.15+
- C++20 compatible compiler (see [main README](../../README.md#platform-support) for details)

## Setup Instructions

### 1. Create Local dotenv Package

First, build and create the dotenv package locally:

```bash
# Navigate to dotenv-cpp root directory
cd /path/to/dotenv-cpp

# Create the Conan package
conan create . --build=missing
```

### 2. Build This Example

```bash
# Navigate to this example directory
cd examples/conan_usage

# Install dependencies
conan install . --output-folder=build --build=missing

# Configure and build
cmake --preset conan-default
cmake --build --preset conan-release
```

### 3. Run the Example

```bash
# Run the executable
./build/Release/dotenv_example
```

## Expected Output

```
=== dotenv-cpp Conan Example ===
Created example.env file
Loaded 3 variables from example.env
Retrieved values:
EXAMPLE_KEY: Hello from dotenv-cpp!
DATABASE_URL: postgresql://localhost:5432/mydb
API_TOKEN: secret-token-123
NUMBER_VALUE (as int): 42
Has EXAMPLE_KEY: yes
Has MISSING_KEY: no
=== Example completed successfully! ===
```

## Project Structure

```
conan_usage/
├── CMakeLists.txt      # CMake configuration
├── conanfile.txt       # Conan dependencies
├── main.cpp           # Example source code
└── README.md          # This file
```

## Key Files Explained

### conanfile.txt
Defines the dotenv dependency and generators for CMake integration.

### CMakeLists.txt
Standard CMake configuration that:
- Finds the dotenv package
- Links against `dotenv::dotenv_lib`
- Sets C++20 requirement

### main.cpp
Demonstrates core dotenv-cpp functionality:
- Setting and saving environment variables
- Loading from .env files
- Type-safe value retrieval
- Existence checking

## Troubleshooting

**Package not found?**
Make sure you've created the local package first:
```bash
conan create /path/to/dotenv-cpp --build=missing
```

**Build errors?**
Ensure your compiler supports C++20:
```bash
# Check compiler version
g++ --version    # GCC 10+
clang++ --version # Clang 12+
```

**Different build configurations?**
```bash
# Debug build
conan install . -s build_type=Debug --output-folder=build-debug --build=missing

# With sanitizers (requires local package built with sanitizers)
conan create /path/to/dotenv-cpp -o enable_sanitizers=True --build=missing
conan install . -o dotenv:enable_sanitizers=True --output-folder=build-debug --build=missing
```
