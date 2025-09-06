#include "dotenv.hpp"
#include <cstdlib>
#include <iostream>

void test_process_env_functions() {
    std::cout << "=== 🔄 Testando apply_internal_to_process_env() ===\n\n";

    // 1. Carregamento interno (sem aplicar ao processo)
    {
        std::cout << "1. 📋 Carregamento interno (apply_system_env=false):\n";

        // Usar a nova API com load_options
        dotenv::load_options opts{
            .overwrite_policy = dotenv::overwrite::replace,
            .apply_to_process = dotenv::process_env_apply::no,
            .backend = dotenv::parse_backend::auto_detect};

        auto [error, count] = dotenv::load("tests/test.env", opts);

        if (error == dotenv::dotenv_error::success) {
            std::cout << "   ✅ Carregadas " << count
                      << " variáveis internamente\n";
        } else {
            std::cout << "   ❌ Erro: " << static_cast<int>(error) << "\n";
        }

        // Verificar que ainda não está no processo
        const char *test_var = std::getenv("TEST_VAR");
        if (test_var == nullptr) {
            std::cout << "   ✅ Variável ainda não está no processo ambiente\n";
        } else {
            std::cout << "   ⚠️  Variável já estava no processo: " << test_var
                      << "\n";
        }

        // Verificar que está no armazenamento interno
        auto internal_value = dotenv::get("TEST_VAR");
        if (!internal_value.empty()) {
            std::cout << "   ✅ Variável disponível internamente: "
                      << internal_value << "\n";
        }
    }

    // 2. Aplicação manual ao processo
    {
        std::cout << "\n2. 🚀 Aplicando variáveis internas ao processo:\n";
        dotenv::apply_internal_to_process_env(dotenv::overwrite::replace);
        std::cout << "   ✅ apply_internal_to_process_env() executado\n";

        // Verificar que agora está no processo
        const char *test_var = std::getenv("TEST_VAR");
        if (test_var != nullptr) {
            std::cout << "   ✅ Variável agora no processo ambiente: "
                      << test_var << "\n";
        } else {
            std::cout << "   ❌ Variável não foi aplicada ao processo\n";
        }
    }

    // 3. Teste da função deprecated (deve gerar warning)
    {
        std::cout << "\n3. ⚠️  Testando compatibilidade backward:\n";
        std::cout << "   ℹ️  Usando a API clássica de load() com parâmetros "
                     "separados\n";

        // Esta é a API clássica que ainda funciona
        int legacy_result = dotenv::load("tests/test.env", 1, true);
        if (legacy_result > 0) {
            std::cout << "   ✅ API clássica ainda funciona: " << legacy_result
                      << " variáveis carregadas\n";
        } else {
            std::cout << "   ❌ API clássica falhou\n";
        }
    }

    // 4. Demonstração de uso recomendado
    {
        std::cout << "\n4. 🎯 Padrão recomendado - carregamento diferido:\n";

        // Carregar configurações diferentes sem aplicar
        dotenv::set("STAGE", "development", dotenv::overwrite::replace);
        dotenv::set("DEBUG_MODE", "true", dotenv::overwrite::replace);
        dotenv::set("LOG_LEVEL", "verbose", dotenv::overwrite::replace);

        std::cout << "   📝 Configurações preparadas internamente\n";
        std::cout << "   🔄 Validando configurações...\n";

        // Simular validação
        bool config_valid = dotenv::get("STAGE") == "development" &&
                            dotenv::get("DEBUG_MODE") == "true";

        if (config_valid) {
            std::cout
                << "   ✅ Configurações válidas, aplicando ao processo...\n";
            dotenv::apply_internal_to_process_env(dotenv::overwrite::replace);

            // Verificar aplicação
            const char *stage = std::getenv("STAGE");
            const char *debug = std::getenv("DEBUG_MODE");

            if (stage != nullptr && debug != nullptr) {
                std::cout << "   ✅ Processo configurado: STAGE=" << stage
                          << ", DEBUG_MODE=" << debug << "\n";
            }
        } else {
            std::cout << "   ❌ Configurações inválidas, não aplicando\n";
        }
    }

    std::cout
        << "\n=== ✨ Teste de apply_internal_to_process_env() concluído! ===\n";
}

int main() {
    std::cout << "🎯 Demonstração da Nova API de Processo\n";
    std::cout << "Função: apply_internal_to_process_env()\n";
    std::cout << "Compilador: " <<
#ifdef __clang__
        "Clang " << __clang_major__ << "." << __clang_minor__ << "\n";
#elif defined(__GNUC__)
        "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "\n";
#else
        "Unknown\n";
#endif
    std::cout << "C++: " << __cplusplus << "\n\n";

    test_process_env_functions();

    return 0;
}
