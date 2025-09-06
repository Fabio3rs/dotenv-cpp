/**
 * @file example_cpp_error_handling.cpp
 * @brief Example demonstrating C++ error handling with dotenv_error_t enum
 *
 * This example shows how to use the new C++ functions that return
 * std::pair<dotenv_error_t, int> for detailed error reporting.
 */

#include "dotenv.hpp"
#include "dotenv_errors.h"
#include <iostream>
#include <string>

int main() {
    // Example 1: Load with detailed status information
    std::cout << "=== Example 1: Loading with detailed status ===" << std::endl;

    auto [status, variables_loaded] = dotenv::load_with_status(".env", 1, true);

    if (status == DOTENV_SUCCESS) {
        std::cout << "✅ Successfully loaded " << variables_loaded
                  << " variables" << std::endl;
    } else {
        std::cout << "❌ Error: " << dotenv_get_error_message(status)
                  << std::endl;
        std::cout << "   Variables loaded before error: " << variables_loaded
                  << std::endl;
    }

    // Example 2: Force traditional implementation with status
    std::cout << "\n=== Example 2: Traditional implementation ===" << std::endl;

    auto [trad_status, trad_count] =
        dotenv::load_traditional_with_status("config.env", 0, false);

    switch (trad_status) {
    case DOTENV_SUCCESS:
        std::cout << "✅ Traditional load: " << trad_count
                  << " variables loaded" << std::endl;
        break;
    case DOTENV_ERROR_FILE_NOT_FOUND:
        std::cout
            << "⚠️  File not found - this is expected for optional config files"
            << std::endl;
        break;
    case DOTENV_ERROR_PERMISSION_DENIED:
        std::cout << "❌ Permission denied accessing config file" << std::endl;
        break;
    default:
        std::cout << "❌ Error: " << dotenv_get_error_message(trad_status)
                  << std::endl;
    }

#ifdef DOTENV_SIMD_ENABLED
    // Example 3: SIMD implementation with status (if available)
    std::cout << "\n=== Example 3: SIMD implementation ===" << std::endl;

    auto [simd_status, simd_count] =
        dotenv::load_simd_with_status("large.env", 1, true);

    if (simd_status == DOTENV_SUCCESS) {
        std::cout << "🚀 SIMD load: " << simd_count << " variables loaded"
                  << std::endl;
    } else {
        std::cout << "❌ SIMD error: " << dotenv_get_error_message(simd_status)
                  << std::endl;
    }
#endif

    // Example 4: Error handling in loops
    std::cout << "\n=== Example 4: Batch loading with error handling ==="
              << std::endl;

    const std::vector<std::string> config_files = {
        ".env",           // Main config
        ".env.local",     // Local overrides
        ".env.production" // Production config
    };

    int total_variables = 0;
    for (const auto &file : config_files) {
        auto [file_status, file_count] =
            dotenv::load_with_status(file, 1, false);

        switch (file_status) {
        case DOTENV_SUCCESS:
            std::cout << "✅ " << file << ": " << file_count << " variables"
                      << std::endl;
            total_variables += file_count;
            break;
        case DOTENV_ERROR_FILE_NOT_FOUND:
            std::cout << "⚠️  " << file << ": file not found (skipping)"
                      << std::endl;
            break;
        default:
            std::cout << "❌ " << file << ": "
                      << dotenv_get_error_message(file_status) << std::endl;
        }
    }

    std::cout << "📊 Total variables loaded: " << total_variables << std::endl;

    // Apply all loaded variables to system environment
    dotenv::apply_internal_to_process_env(1);

    return 0;
}
