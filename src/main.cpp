#include "dotenv.hpp"
#include <iostream>

auto main() -> int {
    std::cout << "Hello, World!" << '\n';

    dotenv::load();

    std::cout << dotenv::get("FOO") << '\n';

    return 0;
}
