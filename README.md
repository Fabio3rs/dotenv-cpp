# dotenv

A lightweight C++ library for managing `.env` files, designed for easy integration into your projects. The library supports both C++ and C interfaces, allowing you to load, read, write, and manage environment variables stored in `.env` files.

🚧 Disclaimer: This project is currently in development (Alpha/Study phase).
The API, functionality, and implementation details are subject to change. Use it for learning, experimentation, or early feedback. Not recommended for production use.

## Features

- Load environment variables from `.env` files.
- Retrieve environment variables with default values.
- Check for the existence of environment variables.
- Modify and save environment variables.
- C++ and C compatibility for broader use cases.

---

## Installation

### Using Conan

You can integrate `dotenv` into your project using Conan:

1. Add `dotenv` to your `conanfile.txt`:
   ```plaintext
   [requires]
   dotenv/0.1
   ```

2. Install the dependencies:
   ```bash
   conan install . --output-folder=build --build=missing
   ```

3. Link the library in your `CMakeLists.txt`:
   ```cmake
   find_package(dotenv REQUIRED)
   target_link_libraries(my_project PRIVATE dotenv::dotenv_lib)
   ```

### Manual Integration

1. Clone the repository:
   ```bash
   git clone https://github.com/Fabio3rs/dotenv-cpp.git
   ```

2. Include the `include` directory in your project's include paths.

3. Add the `src` files to your build system.

---

## Usage

### C++ API

Include `dotenv.hpp` and use the `dotenv` namespace:

```cpp
#include "dotenv.hpp"
#include <iostream>

int main() {
    dotenv::load(".env");

    std::string_view value = dotenv::get("MY_KEY", "default_value");
    std::cout << "MY_KEY: " << value << std::endl;

    dotenv::set("NEW_KEY", "new_value");
    dotenv::save(".env");

    return 0;
}
```

### C API

Include `dotenv.h` and use the C functions:

```c
#include "dotenv.h"
#include <stdio.h>

int main() {
    dotenv_load(".env", 1);

    const char *value = dotenv_get("MY_KEY", "default_value");
    printf("MY_KEY: %s\n", value);

    return 0;
}
```

---

## API Reference

### C++ API (`dotenv.hpp`)

- **`int dotenv::load(std::string_view path = ".env", int replace = 1)`**
  Loads the environment variables from the specified `.env` file.
  - `path`: Path to the `.env` file (default: `.env`).
  - `replace`: Replace existing variables (default: `1`).

- **`std::string_view dotenv::get(std::string_view key, std::string_view default_value = "")`**
  Retrieves the value of the given key or a default value if the key doesn't exist.

- **`bool dotenv::has(std::string_view key)`**
  Checks if a key exists.

- **`void dotenv::set(std::string_view key, std::string_view value, bool replace = true)`**
  Sets a key-value pair. Replaces the value if the key already exists and `replace` is true.

- **`void dotenv::save(std::string_view path)`**
  Saves the current environment variables to the specified `.env` file.

### C API (`dotenv.h`)

- **`int dotenv_load(const char *path = ".env", int replace = 1)`**
  Loads environment variables from a `.env` file.

- **`const char *dotenv_get(const char *key, const char *default_value = "")`**
  Retrieves the value of the given key or a default value if the key doesn't exist.

---

## Building from Source

### Requirements

- A C++20-compliant compiler.
- CMake 3.12 or later.

### Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/Fabio3rs/dotenv-cpp.git
   cd dotenv
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

## Contributing

Contributions are welcome! Please submit issues and pull requests to improve the library.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
