#include "dotenv.hpp"
#include <iostream>

void test_api_compatibility() {
    std::cout << "=== Testando compatibilidade de APIs ===\n\n";

    // 1. API clássica (sempre disponível)
    {
        std::cout << "1. API clássica (retorna int):\n";
        // structured binding expects pair style API
        auto [error, result] = dotenv::load_legacy("tests/test.env");
        if (error == dotenv::dotenv_error::success && result > 0) {
            std::cout << "   ✅ Carregadas " << result << " variáveis\n";
        } else {
            std::cout << "   ❌ Erro: " << static_cast<int>(error) << "\n";
        }
    }

    // 2. API com std::pair (C++11+)
    {
        std::cout << "\n2. API com std::pair<error, value>:\n";
        auto [error, count] =
            dotenv::load_with_status("tests/test.env", 1, true);
        if (error == DOTENV_SUCCESS) {
            std::cout << "   ✅ Sucesso: " << count
                      << " variáveis carregadas\n";
        } else {
            std::cout << "   ❌ Erro: " << dotenv_get_error_message(error)
                      << "\n";
        }
    }

#if DOTENV_HAS_STD_EXPECTED
    // 3. API moderna com std::expected (C++23)
    {
        std::cout << "\n3. API moderna com std::expected:\n";
        // Use the modern std::expected API
        auto result = dotenv::load("tests/test.env", {});
        if (result.has_value()) {
            std::cout << "   ✅ Sucesso: " << result.value()
                      << " variáveis carregadas\n";

            // Teste de programação funcional: transformar o valor
            auto doubled =
                result.transform([](int count) { return count * 2; });
            if (doubled.has_value()) {
                std::cout << "   📊 Transform: " << doubled.value()
                          << " (dobrado)\n";
            }
        } else {
            std::cout << "   ❌ Erro: "
                      << dotenv_get_error_message(
                             static_cast<dotenv_error_t>(result.error()))
                      << "\n";
        }

        // Teste de fallback com or_else
        auto fallback_result =
            dotenv::load("arquivo_inexistente.env", {})
                .or_else([](dotenv::dotenv_error) {
                    std::cout << "   🔧 Usando fallback...\n";
                    return dotenv::load("tests/test.env", {});
                });

        if (fallback_result.has_value()) {
            std::cout << "   ✅ Fallback bem-sucedido: "
                      << fallback_result.value() << " variáveis\n";
        }

        // Teste de value_or
        int safe_count =
            dotenv::load("arquivo_inexistente.env", {}).value_or(0);
        std::cout << "   📝 Value_or: " << safe_count << " (valor padrão)\n";
    }
#else
    std::cout << "\n3. std::expected não disponível:\n";
    std::cout << "   ℹ️  Compilador: " <<
#ifdef __clang__
        "Clang " << __clang_major__ << "." << __clang_minor__ << "\n";
#elif defined(__GNUC__)
        "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "\n";
#else
        "Unknown\n";
#endif
    std::cout << "   ℹ️  C++: " << __cplusplus << "\n";
    std::cout << "   ℹ️  Requer: C++23 com std::expected (GCC 14+, Clang 15+)\n";
#endif

    std::cout << "\n=== Teste de compatibilidade concluído! ===\n";
}

auto main() -> int {
    std::cout << "🚀 Testando APIs de tratamento de erro\n";
#ifdef __clang__
    std::cout << "Compilador: Clang " << __clang_major__ << "."
              << __clang_minor__ << "\n";
#elif defined(__GNUC__)
    std::cout << "Compilador: GCC " << __GNUC__ << "." << __GNUC_MINOR__
              << "\n";
#else
    std::cout << "Compilador: Desconhecido\n";
#endif
    std::cout << "C++: " << __cplusplus << "\n\n";

    test_api_compatibility();

    return 0;
}
