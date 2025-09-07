#include <dotenv.hpp>
#include <iostream>
#include <string>

int main() {
    std::cout << "=== dotenv-cpp Basic Usage Example ===" << std::endl;
    std::cout << std::endl;

    // 1. Carregar arquivo .env
    std::cout << "1. Loading example.env file..." << std::endl;
    auto [error, loaded_vars] = dotenv::load_legacy("example.env");
    if (error != dotenv::dotenv_error::success) {
        std::cerr << "Failed to load example.env file! Error: "
                  << static_cast<int>(error) << '\n';
        return 1;
    }
    std::cout << "   Loaded " << loaded_vars << " variables\n";
    std::cout << '\n';

    // 2. Leitura básica de strings
    std::cout << "2. Reading string values:\n";
    std::cout << "   APP_NAME: " << dotenv::get("APP_NAME", "Unknown") << '\n';
    std::cout << "   APP_VERSION: " << dotenv::get("APP_VERSION", "0.0.0")
              << '\n';
    std::cout << "   WELCOME_MESSAGE: "
              << dotenv::get("WELCOME_MESSAGE", "Hello!") << '\n';
    std::cout << '\n';

    // 3. Leitura de valores numéricos
    std::cout << "3. Reading numeric values:\n";

    // Usando value_or template com tipo específico
    auto db_port = dotenv::value_or<int>("DB_PORT", 3306);
    auto max_connections = dotenv::value_or<int>("MAX_CONNECTIONS", 50);
    auto pi_value = dotenv::value_or<double>("PI_VALUE", 3.14);

    std::cout << "   DB_PORT: " << db_port << '\n';
    std::cout << "   MAX_CONNECTIONS: " << max_connections << '\n';
    std::cout << "   PI_VALUE: " << pi_value << '\n';
    std::cout << '\n';

    // 4. Usando APIs opcionais (C++20)
    std::cout << "4. Using optional APIs:\n";

    auto timeout = dotenv::try_value<int>("TIMEOUT_MS");
    if (timeout) {
        std::cout << "   TIMEOUT_MS: " << *timeout << "ms\n";
    } else {
        std::cout << "   TIMEOUT_MS: not set\n";
    }

    auto optional_setting = dotenv::try_value("OPTIONAL_SETTING");
    if (optional_setting) {
        std::cout << "   OPTIONAL_SETTING: '" << *optional_setting << "'\n";
    } else {
        std::cout << "   OPTIONAL_SETTING: not set\n";
    }
    std::cout << '\n';

    // 5. Verificar existência de chaves
    std::cout << "5. Checking key existence:\n";
    std::cout << "   DEBUG exists: "
              << (dotenv::contains("DEBUG") ? "yes" : "no") << '\n';
    std::cout << "   NONEXISTENT exists: "
              << (dotenv::contains("NONEXISTENT") ? "yes" : "no") << '\n';
    std::cout << '\n';

    // 6. Definir novas variáveis
    std::cout << "6. Setting new variables:\n";
    dotenv::set("RUNTIME_VALUE", "set_from_cpp");
    dotenv::set("CURRENT_USER", "example_user");

    std::cout << "   RUNTIME_VALUE: " << dotenv::get("RUNTIME_VALUE", "not_set")
              << '\n';
    std::cout << "   CURRENT_USER: " << dotenv::get("CURRENT_USER", "unknown")
              << '\n';
    std::cout << '\n';

    // 7. Demonstrar comportamento com replace
    std::cout << "7. Testing replace behavior:\n";

    // Primeira definição
    dotenv::set("TEST_REPLACE", "original_value");
    std::cout << "   Initial value: " << dotenv::get("TEST_REPLACE") << '\n';

    // Tentar substituir com preserve (não deve alterar)
    dotenv::set("TEST_REPLACE", "new_value", dotenv::overwrite::preserve);
    std::cout << "   After set with preserve: " << dotenv::get("TEST_REPLACE")
              << '\n';

    // Substituir com replace (deve alterar)
    dotenv::set("TEST_REPLACE", "replaced_value", dotenv::overwrite::replace);
    std::cout << "   After set with replace: " << dotenv::get("TEST_REPLACE")
              << '\n';
    std::cout << '\n';

    // 8. Salvar variáveis (apenas as gerenciadas)
    std::cout << "8. Saving managed variables to output.env...\n";
    dotenv::save_to_file("output.env");
    std::cout << "   Variables saved to output.env\n";
    std::cout << '\n';

    std::cout << "=== Example completed successfully! ===\n";
    return 0;
}
