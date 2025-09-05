#include <dotenv.hpp>
#include <iostream>

int main() {
    std::cout << "=== CMake Integration Example ===" << std::endl;

    // Carregar configurações
    if (dotenv::load("config.env") < 0) {
        std::cerr << "Failed to load config.env" << std::endl;
        return 1;
    }

    // Mostrar configurações carregadas
    std::cout << "Project: " << dotenv::get("PROJECT_NAME", "Unknown")
              << std::endl;
    std::cout << "Build Type: " << dotenv::get("BUILD_TYPE", "Debug")
              << std::endl;
    std::cout << "Target Arch: " << dotenv::get("TARGET_ARCH", "unknown")
              << std::endl;

    // Configurações booleanas
    bool warnings = dotenv::get("ENABLE_WARNINGS", "false") == "true";
    std::cout << "Warnings: " << (warnings ? "enabled" : "disabled")
              << std::endl;

    // Configurações numéricas
    auto opt_level = dotenv::get<int>("OPTIMIZATION_LEVEL", 0);
    std::cout << "Optimization Level: " << opt_level << std::endl;

    std::cout << "CMake integration working correctly!" << std::endl;
    return 0;
}
