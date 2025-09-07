#include "dotenv.hpp"
#include <iostream>

#if __has_include(<expected>) && __cplusplus >= 202302L
#include <expected>
#define HAS_EXPECTED 1
#else
#define HAS_EXPECTED 0
#endif

#if __has_include(<format>) && __cplusplus >= 202002L
#include <format>
#define HAS_FORMAT 1
#else
#include <sstream>
#define HAS_FORMAT 0
#endif

void demonstrate_cpp23_expected() {
    std::cout << "⚡ Modern C++23 std::expected Example\n\n";

#if HAS_EXPECTED && defined(DOTENV_HAS_EXPECTED)
    // 1. Basic std::expected usage
    {
        std::cout << "1. 🎯 std::expected basic usage:\n";

        auto result = dotenv::load("app.env", {});
        if (result) {
#if HAS_FORMAT
            std::cout << std::format("   ✅ Loaded {} variables\n", *result);
#else
            std::cout << "   ✅ Loaded " << *result << " variables\n";
#endif
        } else {
#if HAS_FORMAT
            std::cout << std::format("   ❌ Error: {}\n",
                                     static_cast<int>(result.error()));
#else
            std::cout << "   ❌ Error: " << static_cast<int>(result.error())
                      << "\n";
#endif
        }
    }

    // 2. Monadic operations with transform
    {
        std::cout << "\n2. 🔗 Monadic operations:\n";

        auto pipeline =
            dotenv::load("app.env", {})
                .transform([](int count) {
                    std::cout << "   🔄 Transform: Processing " << count
                              << " variables\n";
                    return count * 2; // Double the count for demonstration
                })
                .transform([](int doubled) {
                    std::cout << "   📊 Transform: Doubled count = " << doubled
                              << "\n";
                    return doubled;
                });

        if (pipeline) {
#if HAS_FORMAT
            std::cout << std::format("   ✅ Pipeline result: {}\n", *pipeline);
#else
            std::cout << "   ✅ Pipeline result: " << *pipeline << "\n";
#endif
        }
    }

    // 3. Error handling with or_else
    {
        std::cout << "\n3. 🛡️  Robust error handling with fallback:\n";

        auto robust_load =
            dotenv::load("nonexistent.env", {})
                .or_else([](auto error)
                             -> std::expected<int, dotenv::dotenv_error> {
                    std::cout
                        << "   🔧 Primary file failed, trying fallback...\n";
                    return dotenv::load("fallback.env", {});
                })
                .or_else(
                    [](auto error) -> std::expected<int, dotenv::dotenv_error> {
                        std::cout
                            << "   🔧 Fallback failed, using app.env...\n";
                        return dotenv::load("app.env", {});
                    });

        if (robust_load) {
#if HAS_FORMAT
            std::cout << std::format(
                "   ✅ Robust load successful: {} variables\n", *robust_load);
#else
            std::cout << "   ✅ Robust load successful: " << *robust_load
                      << " variables\n";
#endif
        } else {
            std::cout << "   ❌ All fallbacks failed\n";
        }
    }

    // 4. Advanced functional composition
    {
        std::cout << "\n4. 🚀 Advanced functional composition:\n";

        auto validate_config =
            [](int count) -> std::expected<std::string, dotenv::dotenv_error> {
            if (count == 0) {
                return std::unexpected(dotenv::dotenv_error::file_not_found);
            }
            if (count < 5) {
                return std::unexpected(dotenv::dotenv_error::invalid_format);
            }
            return std::string("Configuration is valid");
        };

        auto config_result =
            dotenv::load("app.env", {})
                .and_then(
                    [&](int count)
                        -> std::expected<std::string, dotenv::dotenv_error> {
                        std::cout << "   🔍 Validating configuration...\n";
                        return validate_config(count);
                    });

        if (config_result) {
#if HAS_FORMAT
            std::cout << std::format("   ✅ {}\n", *config_result);
#else
            std::cout << "   ✅ " << *config_result << "\n";
#endif
        } else {
#if HAS_FORMAT
            std::cout << std::format("   ❌ Validation failed: {}\n",
                                     static_cast<int>(config_result.error()));
#else
            std::cout << "   ❌ Validation failed: "
                      << static_cast<int>(config_result.error()) << "\n";
#endif
        }
    }

    // 5. Value extraction with safe defaults
    {
        std::cout << "\n5. 🎯 Safe value extraction:\n";

        // Using value_or for safe defaults
        auto safe_count = dotenv::load("missing.env", {}).value_or(0);
#if HAS_FORMAT
        std::cout << std::format("   📊 Safe count with default: {}\n",
                                 safe_count);
#else
        std::cout << "   📊 Safe count with default: " << safe_count << "\n";
#endif

        // Chaining with error handling
        auto app_name =
            dotenv::load("app.env", {})
                .transform([](int) { return dotenv::try_value("APP_NAME"); })
                .value_or(std::optional<std::string>{});

        if (app_name.has_value()) {
#if HAS_FORMAT
            std::cout << std::format("   🏷️  App name: {}\n", *app_name);
#else
            std::cout << "   🏷️  App name: " << *app_name << "\n";
#endif
        } else {
            std::cout << "   ⚠️  App name not configured\n";
        }
    }

#else
    std::cout << "❌ std::expected not available\n";
    std::cout << "This example requires:\n";
    std::cout << "- C++23 compiler support\n";
    std::cout << "- std::expected implementation\n";
    std::cout << "- DOTENV_HAS_EXPECTED=1\n\n";

    std::cout << "Current status:\n";
#if HAS_EXPECTED
    std::cout << "✅ <expected> header available\n";
#else
    std::cout << "❌ <expected> header not found\n";
#endif

#ifdef DOTENV_HAS_EXPECTED
    std::cout << "✅ DOTENV_HAS_EXPECTED defined\n";
#else
    std::cout << "❌ DOTENV_HAS_EXPECTED not defined\n";
#endif

    std::cout << "C++ Standard: " << __cplusplus << "\n";
#endif
}

int main() {
    demonstrate_cpp23_expected();

    std::cout << "\n🎉 Modern C++23 example completed!\n";
    return 0;
}
