#include "dotenv.hpp"
#include <array>
#include <chrono>
#include <iostream>
#include <string_view>

#if __has_include(<format>)
#include <format>
#define HAS_FORMAT 1
#else
#include <sstream>
#define HAS_FORMAT 0
#endif

using namespace std::chrono_literals;
using namespace std::string_view_literals;

void demonstrate_modern_api() {
    std::cout << "🚀 Modern C++20 dotenv-cpp Example\n\n";

    // 1. Modern load with structured bindings (C++17) and load_options (C++20)
    {
        std::cout << "1. 📋 Structured bindings + load_options:\n";

        dotenv::load_options opts{
            .overwrite_policy = dotenv::overwrite::replace,
            .apply_to_process = dotenv::process_env_apply::yes,
            .backend = dotenv::parse_backend::auto_detect};

        auto [error, count] = dotenv::load_legacy("config.env", opts);

        if (error == dotenv::dotenv_error::success) {
            std::cout << std::format("   ✅ Loaded {} variables\n", count);
        } else {
            std::cout << std::format("   ❌ Error: {}\n",
                                     static_cast<int>(error));
        }
    }

    // 2. Type-safe value extraction with concepts (C++20)
    {
        std::cout << "\n2. 🎯 Type-safe value extraction:\n";

        // Modern string literals and value_or
        constexpr int default_port = 8080;
        auto port = dotenv::value_or<int>("SERVER_PORT", default_port);
        auto host = dotenv::value("SERVER_HOST", "localhost"sv);

#if HAS_FORMAT
        std::cout << std::format("   🌐 Server: {}:{}\n", host, port);
#else
        std::cout << "   🌐 Server: " << host << ":" << port << "\n";
#endif
    }

    // 3. Modern error handling patterns
    {
        std::cout << "\n3. ⚡ Modern error handling:\n";

        auto database_url = dotenv::try_value("DATABASE_URL");
        if (database_url) {
#if HAS_FORMAT
            std::cout << std::format("   ✅ Database: {}\n", *database_url);
#else
            std::cout << "   ✅ Database: " << *database_url << "\n";
#endif
        } else {
            std::cout << "   ⚠️  No database URL configured\n";
        }

        // Value with validation
        try {
            auto max_connections =
                dotenv::value_required<int>("MAX_CONNECTIONS");
#if HAS_FORMAT
            std::cout << std::format("   📊 Max connections: {}\n",
                                     max_connections);
#else
            std::cout << "   📊 Max connections: " << max_connections << "\n";
#endif
        } catch (const std::invalid_argument &e) {
#if HAS_FORMAT
            std::cout << std::format("   ❌ Config error: {}\n", e.what());
#else
            std::cout << "   ❌ Config error: " << e.what() << "\n";
#endif
        }
    }

    // 4. Range-based operations with views (C++20)
    {
        std::cout << "\n4. 🔍 Modern configuration inspection:\n";

        // Check if we have debug mode enabled
        constexpr std::array debug_keys{"DEBUG"sv, "DEBUG_MODE"sv,
                                        "DEVELOPMENT"sv, "DEV_MODE"sv};

        bool debug_enabled = false;
        for (auto key : debug_keys) {
            if (dotenv::contains(key)) {
                auto value = dotenv::get(key);
                if (value == "true" || value == "1" || value == "yes") {
                    debug_enabled = true;
#if HAS_FORMAT
                    std::cout
                        << std::format("   🐛 Debug enabled via {}\n", key);
#else
                    std::cout << "   🐛 Debug enabled via " << key << "\n";
#endif
                    break;
                }
            }
        }

        if (!debug_enabled) {
            std::cout << "   ✅ Production mode active\n";
        }
    }

    // 5. SIMD performance (if available)
    {
        std::cout << "\n5. ⚡ Performance features:\n";

#ifdef DOTENV_SIMD_ENABLED
        std::cout << "   🏃 SIMD optimizations: ✅ Available\n";

        auto start = std::chrono::high_resolution_clock::now();
        [[maybe_unused]] auto [error, count] = dotenv::load_simd_legacy(
            "production.env", {.backend = dotenv::parse_backend::simd});
        auto end = std::chrono::high_resolution_clock::now();

        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start);
#if HAS_FORMAT
        std::cout << std::format("   ⏱️  SIMD load time: {}μs\n",
                                 duration.count());
#else
        std::cout << "   ⏱️  SIMD load time: " << duration.count() << "μs\n";
#endif
#else
        std::cout << "   🏃 SIMD optimizations: ❌ Not available\n";
#endif
    }
}

int main() {
    demonstrate_modern_api();

    std::cout << "\n🎉 Modern C++20 example completed!\n";
    return 0;
}
