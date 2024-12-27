#include "dotenv.hpp"
#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;

    dotenv::load();

    std::cout << dotenv::get("FOO") << std::endl;

    return 0;
}
