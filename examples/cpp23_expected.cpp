/**
 * @file example_cpp23_expected.cpp
 * @brief Example demonstrating C++23 std::expected with dotenv
 *
 * This example shows how to use the new C++23 std::expected functions
 * for modern, idiomatic error handling in dotenv.
 */

#include "../include/dotenv.hpp"
#include "../include/dotenv_errors.h"
#include <iostream>
#include <string>
#include <vector>

// Only compile this example with C++23 or later
#if __cplusplus >= 202302L

int main() {
    std::cout << "=== Dotenv C++23 std::expected Examples ===" << '\n';

    // Example 1: Basic usage with std::expected
    std::cout << "\n1. Basic std::expected usage:" << '\n';

    auto result = dotenv::load_expected(".env", 1, false);

    if (result) {
        std::cout << "✅ Successfully loaded " << *result << " variables"
                  << '\n';

        // Access environment variables
        auto db_host = dotenv::get("DB_HOST", "localhost");
        auto db_port =
            dotenv::get<int>(dotenv::NoThrowPolicy{}, "DB_PORT", 5432);

        std::cout << "   DB_HOST: " << db_host << '\n';
        std::cout << "   DB_PORT: " << db_port << '\n';

    } else {
        std::cout << "❌ Failed to load: "
                  << dotenv_get_error_message(result.error()) << '\n';
    }

    // Example 2: Chaining with monadic operations
    std::cout << "\n2. Monadic operations with std::expected:" << '\n';

    auto process_config =
        [](int variables_loaded) -> std::expected<std::string, dotenv_error_t> {
        if (variables_loaded == 0) {
            return std::unexpected(DOTENV_ERROR_INVALID_FORMAT);
        }
        return "Configuration processed with " +
               std::to_string(variables_loaded) + " variables";
    };

    auto config_result =
        dotenv::load_expected("nonexistent.env", 1, false)
            .and_then(process_config)
            .or_else([](dotenv_error_t error)
                         -> std::expected<std::string, dotenv_error_t> {
                if (error == DOTENV_ERROR_FILE_NOT_FOUND) {
                    return "Using default configuration (no .env file found)";
                }
                return std::unexpected(error);
            });

    if (config_result) {
        std::cout << "✅ " << *config_result << '\n';
    } else {
        std::cout << "❌ Config error: "
                  << dotenv_get_error_message(config_result.error()) << '\n';
    }

    // Example 3: Traditional implementation with std::expected
    std::cout << "\n3. Traditional implementation:" << '\n';

    auto trad_result = dotenv::load_traditional_expected(".env", 0, false);

    // Pattern matching style error handling
    auto handle_result = [](const auto &res, const std::string &method) {
        if (res.has_value()) {
            std::cout << "✅ " << method << ": " << res.value()
                      << " variables loaded" << '\n';
        } else {
            switch (res.error()) {
            case DOTENV_ERROR_FILE_NOT_FOUND:
                std::cout << "⚠️  " << method << ": File not found (optional)"
                          << '\n';
                break;
            case DOTENV_ERROR_PERMISSION_DENIED:
                std::cout << "❌ " << method << ": Permission denied" << '\n';
                break;
            default:
                std::cout << "❌ " << method << ": "
                          << dotenv_get_error_message(res.error()) << '\n';
            }
        }
    };

    handle_result(trad_result, "Traditional");

#ifdef DOTENV_SIMD_ENABLED
    // Example 4: SIMD implementation with std::expected
    std::cout << "\n4. SIMD implementation:" << '\n';

    auto simd_result = dotenv::load_simd_expected(".env", 1, false);
    handle_result(simd_result, "SIMD");
#endif

    // Example 5: Batch processing with error aggregation
    std::cout << "\n5. Batch processing multiple files:" << '\n';

    const std::vector<std::string> config_files = {".env", ".env.local",
                                                   ".env.production"};

    int total_variables = 0;
    std::vector<std::string> errors;

    for (const auto &file : config_files) {
        auto file_result = dotenv::load_expected(file, 1, false);

        file_result
            .and_then([&](int count) -> std::expected<int, dotenv_error_t> {
                std::cout << "✅ " << file << ": " << count << " variables"
                          << '\n';
                total_variables += count;
                return count;
            })
            .or_else([&](dotenv_error_t error)
                         -> std::expected<int, dotenv_error_t> {
                if (error == DOTENV_ERROR_FILE_NOT_FOUND) {
                    std::cout << "⚠️  " << file << ": not found (skipping)"
                              << '\n';
                } else {
                    std::cout << "❌ " << file << ": "
                              << dotenv_get_error_message(error) << '\n';
                    errors.push_back(file + ": " +
                                     dotenv_get_error_message(error));
                }
                return std::unexpected(error);
            });
    }

    std::cout << "📊 Total variables loaded: " << total_variables << '\n';
    if (!errors.empty()) {
        std::cout << "⚠️  " << errors.size() << " errors encountered" << '\n';
    }

    // Example 6: Using value_or for default handling
    std::cout << "\n6. Using value_or for defaults:" << '\n';

    auto safe_load = [](const std::string &file) {
        return dotenv::load_expected(file, 1, false).value_or(0);
    };

    int main_config = safe_load(".env");
    int local_config = safe_load(".env.local");

    std::cout << "Main config variables: " << main_config << '\n';
    std::cout << "Local config variables: " << local_config << '\n';

    std::cout << "\n✅ All C++23 std::expected examples completed!" << '\n';
    return 0;
}

#else
// Fallback for pre-C++23 compilers
int main() {
    std::cout
        << "❌ This example requires C++23 or later for std::expected support"
        << '\n';
    std::cout << "   Current C++ standard: " << __cplusplus << '\n';
    std::cout << "   Required: 202302L (C++23)" << '\n';
    return 1;
}
#endif
