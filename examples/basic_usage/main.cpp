#include <dotenv.hpp>
#include <iostream>
#include <string>

int main() {
    std::cout << "=== dotenv-cpp Basic Usage Example ===" << std::endl;
    std::cout << std::endl;

    // 1. Carregar arquivo .env
    std::cout << "1. Loading example.env file..." << std::endl;
    int loaded_vars = dotenv::load("example.env");
    if (loaded_vars < 0) {
        std::cerr << "Failed to load example.env file!" << std::endl;
        return 1;
    }
    std::cout << "   Loaded " << loaded_vars << " variables" << std::endl;
    std::cout << std::endl;

    // 2. Leitura básica de strings
    std::cout << "2. Reading string values:" << std::endl;
    std::cout << "   APP_NAME: " << dotenv::get("APP_NAME", "Unknown")
              << std::endl;
    std::cout << "   APP_VERSION: " << dotenv::get("APP_VERSION", "0.0.0")
              << std::endl;
    std::cout << "   WELCOME_MESSAGE: "
              << dotenv::get("WELCOME_MESSAGE", "Hello!") << std::endl;
    std::cout << std::endl;

    // 3. Leitura de valores numéricos
    std::cout << "3. Reading numeric values:" << std::endl;

    // Usando get template com tipo específico
    auto db_port = dotenv::get<int>("DB_PORT", 3306);
    auto max_connections = dotenv::get<int>("MAX_CONNECTIONS", 50);
    auto pi_value = dotenv::get<double>("PI_VALUE", 3.14);

    std::cout << "   DB_PORT: " << db_port << std::endl;
    std::cout << "   MAX_CONNECTIONS: " << max_connections << std::endl;
    std::cout << "   PI_VALUE: " << pi_value << std::endl;
    std::cout << std::endl;

    // 4. Usando APIs opcionais (C++20)
    std::cout << "4. Using optional APIs:" << std::endl;

    auto timeout = dotenv::get_optional<int>("TIMEOUT_MS");
    if (timeout) {
        std::cout << "   TIMEOUT_MS: " << *timeout << "ms" << std::endl;
    } else {
        std::cout << "   TIMEOUT_MS: not set" << std::endl;
    }

    auto optional_setting = dotenv::get_optional_string("OPTIONAL_SETTING");
    if (optional_setting) {
        std::cout << "   OPTIONAL_SETTING: '" << *optional_setting << "'"
                  << std::endl;
    } else {
        std::cout << "   OPTIONAL_SETTING: not set" << std::endl;
    }
    std::cout << std::endl;

    // 5. Verificar existência de chaves
    std::cout << "5. Checking key existence:" << std::endl;
    std::cout << "   DEBUG exists: " << (dotenv::has("DEBUG") ? "yes" : "no")
              << std::endl;
    std::cout << "   NONEXISTENT exists: "
              << (dotenv::has("NONEXISTENT") ? "yes" : "no") << std::endl;
    std::cout << std::endl;

    // 6. Definir novas variáveis
    std::cout << "6. Setting new variables:" << std::endl;
    dotenv::set("RUNTIME_VALUE", "set_from_cpp");
    dotenv::set("CURRENT_USER", "example_user");

    std::cout << "   RUNTIME_VALUE: " << dotenv::get("RUNTIME_VALUE", "not_set")
              << std::endl;
    std::cout << "   CURRENT_USER: " << dotenv::get("CURRENT_USER", "unknown")
              << std::endl;
    std::cout << std::endl;

    // 7. Demonstrar comportamento com replace
    std::cout << "7. Testing replace behavior:" << std::endl;

    // Primeira definição
    dotenv::set("TEST_REPLACE", "original_value");
    std::cout << "   Initial value: " << dotenv::get("TEST_REPLACE")
              << std::endl;

    // Tentar substituir com replace=false (não deve alterar)
    dotenv::set("TEST_REPLACE", "new_value", false);
    std::cout << "   After set with replace=false: "
              << dotenv::get("TEST_REPLACE") << std::endl;

    // Substituir com replace=true (deve alterar)
    dotenv::set("TEST_REPLACE", "replaced_value", true);
    std::cout << "   After set with replace=true: "
              << dotenv::get("TEST_REPLACE") << std::endl;
    std::cout << std::endl;

    // 8. Salvar variáveis (apenas as gerenciadas)
    std::cout << "8. Saving managed variables to output.env..." << std::endl;
    dotenv::save("output.env");
    std::cout << "   Variables saved to output.env" << std::endl;
    std::cout << std::endl;

    std::cout << "=== Example completed successfully! ===" << std::endl;
    return 0;
}
