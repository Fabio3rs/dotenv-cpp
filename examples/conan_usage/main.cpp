#include "dotenv.hpp"
#include <iostream>
#include <string_view>

int main() {
    std::cout << "=== dotenv-cpp Conan Example ===\n";

    // Create a sample .env file
    dotenv::set("EXAMPLE_KEY", "Hello from dotenv-cpp!");
    dotenv::set("DATABASE_URL", "postgresql://localhost:5432/mydb");
    dotenv::set("API_TOKEN", "secret-token-123");
    dotenv::save_to_file("example.env");

    std::cout << "Created example.env file\n";

    // Load the .env file using structured bindings
    auto [error, loaded_vars] = dotenv::load_legacy("example.env");
    if (error != dotenv::dotenv_error::success) {
        std::cerr << "Failed to load example.env (error: "
                  << static_cast<int>(error) << ")\n";
        return 1;
    }
    std::cout << "Loaded " << loaded_vars << " variables from example.env\n";

    // Retrieve and display values
    std::string_view example_value = dotenv::get("EXAMPLE_KEY", "default");
    std::string_view db_url = dotenv::get("DATABASE_URL", "not found");
    std::string_view api_token = dotenv::get("API_TOKEN", "not found");

    std::cout << "\nRetrieved values:\n";
    std::cout << "EXAMPLE_KEY: " << example_value << '\n';
    std::cout << "DATABASE_URL: " << db_url << '\n';
    std::cout << "API_TOKEN: " << api_token << '\n';

    // Test type-safe template functions
    dotenv::set("NUMBER_VALUE", "42");
    const int default_number = 0;
    auto number = dotenv::value_or<int>("NUMBER_VALUE", default_number);
    std::cout << "NUMBER_VALUE (as int): " << number << '\n';

    // Test existence check
    bool has_key = dotenv::contains("EXAMPLE_KEY");
    bool has_missing = dotenv::contains("MISSING_KEY");
    std::cout << "Has EXAMPLE_KEY: " << (has_key ? "yes" : "no") << '\n';
    std::cout << "Has MISSING_KEY: " << (has_missing ? "yes" : "no") << '\n';

    std::cout << "\n=== Example completed successfully! ===\n";
    return 0;
}
