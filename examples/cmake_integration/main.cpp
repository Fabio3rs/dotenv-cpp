#include <dotenv.hpp>
#include <iostream>

int main() {
    std::cout << "=== CMake Integration Example ===\n";

    // Load configuration using structured bindings (C++17)
    auto [error, count] = dotenv::load("config.env");
    if (error != dotenv::dotenv_error::success) {
        std::cerr << "Failed to load config.env (error: "
                  << static_cast<int>(error) << ")\n";
        return 1;
    }

    // Mostrar configurações carregadas
    std::cout << "Loaded " << count << " variables\n";
    std::cout << "Project: " << dotenv::get("PROJECT_NAME", "Unknown") << '\n';
    std::cout << "Build Type: " << dotenv::get("BUILD_TYPE", "Debug") << '\n';
    std::cout << "Target Arch: " << dotenv::get("TARGET_ARCH", "unknown")
              << '\n';

    // Configurações booleanas
    bool warnings = dotenv::get("ENABLE_WARNINGS", "false") == "true";
    std::cout << "Warnings: " << (warnings ? "enabled" : "disabled") << '\n';

    // Configurações numéricas
    const int default_opt_level = 0;
    auto opt_level =
        dotenv::value_or<int>("OPTIMIZATION_LEVEL", default_opt_level);
    std::cout << "Optimization Level: " << opt_level << '\n';

    std::cout << "CMake integration working correctly!\n";
    return 0;
}
