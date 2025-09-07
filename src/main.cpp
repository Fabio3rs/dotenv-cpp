#include "dotenv.hpp"
#include <iostream>

auto main() -> int {
    std::cout << "Hello, World!" << '\n';

    // Use legacy pair-returning API when structured bindings are desired
    auto [error, count] = dotenv::load_legacy(".env");

    if (error == dotenv::dotenv_error::success) {
        std::cout << "Loaded " << count << " variables successfully" << '\n';
    } else {
        std::cout << "Failed to load .env file" << '\n';
    }

    std::cout << dotenv::get("FOO") << '\n';

    return 0;
}
