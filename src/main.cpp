#include "dotenv.hpp"
#include <iostream>

auto main() -> int {
    std::cout << "Hello, World!" << '\n';

    // Use the new C++20 API with type-safe configuration
    auto [error, count] = dotenv::load(".env");

    if (error == dotenv::dotenv_error::success) {
        std::cout << "Loaded " << count << " variables successfully" << '\n';
    } else {
        std::cout << "Failed to load .env file" << '\n';
    }

    std::cout << dotenv::get("FOO") << '\n';

    return 0;
}
