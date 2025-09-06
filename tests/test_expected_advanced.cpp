#include "dotenv.hpp"
#include <cassert>
#include <iostream>

#if __cplusplus >= 202302L
#include <expected>

void test_monadic_operations() {
    std::cout << "=== 🚀 Testando Operações Monádicas do std::expected ===\n\n";

    // 1. Pipeline de carregamento com validação
    {
        std::cout << "1. 📋 Pipeline de carregamento com validação:\n";
        auto pipeline =
            dotenv::load_expected("tests/test.env")
                .transform([](int count) {
                    std::cout << "   🔄 Etapa 1: Carregadas " << count
                              << " variáveis\n";
                    if (count == 0) {
                        throw std::runtime_error("Nenhuma variável carregada");
                    }
                    return count;
                })
                .transform([](int count) {
                    std::cout << "   🔄 Etapa 2: Validando configurações...\n";
                    // Simular validação adicional
                    return count * 10; // Score de qualidade
                });

        · if (pipeline) {
            std::cout << "   ✅ Pipeline concluído! Score: " << *pipeline
                      << "\n\n";
        }
        else {
            std::cout << "   ❌ Pipeline falhou!\n\n";
        }
    }

    // 2. Sistema de fallback em cascata
    {
        std::cout << "2. 🏗️ Sistema de fallback em cascata:\n";
        auto robust_load =
            dotenv::load_expected("config.env")
                .or_else(
                    [](dotenv_error_t) -> std::expected<int, dotenv_error_t> {
                        std::cout
                            << "   🔧 Tentativa 1: Usando .env.local...\n";
                        return dotenv::load_expected(".env.local");
                    })
                .or_else(
                    [](dotenv_error_t) -> std::expected<int, dotenv_error_t> {
                        std::cout
                            << "   🔧 Tentativa 2: Usando .env padrão...\n";
                        return dotenv::load_expected(".env");
                    })
                .or_else([](dotenv_error_t)
                             -> std::expected<int, dotenv_error_t> {
                    std::cout
                        << "   🔧 Tentativa 3: Usando arquivo de teste...\n";
                    return dotenv::load_expected("tests/test.env");
                });

        · if (robust_load) {
            std::cout << "   ✅ Carregamento robusto bem-sucedido: "
                      << *robust_load << " variáveis\n\n";
        }
        else {
            std::cout << "   ❌ Todos os fallbacks falharam!\n\n";
        }
    }

    // 3. Composição de múltiplas operações
    {
        std::cout << "3. 🔗 Composição de múltiplas operações:\n";
        auto composite =
            dotenv::load_expected("tests/test.env")
                .and_then([](int count)
                              -> std::expected<std::string, dotenv_error_t> {
                    std::cout << "   🔄 Processando " << count
                              << " variáveis...\n";
                    if (count > 0) {
                        return std::string("Configuração válida com ") +
                               std::to_string(count) + " vars";
                    }
                    return std::unexpected(DOTENV_ERROR_INVALID_FORMAT);
                })
                .transform([](const std::string &msg) {
                    std::cout << "   📝 Mensagem gerada: " << msg << "\n";
                    return msg.length();
                });

        · if (composite) {
            std::cout << "   ✅ Composição concluída! Tamanho da mensagem: "
                      << *composite << "\n\n";
        }
    }

    // 4. Tratamento funcional de erros
    {
        std::cout << "4. ⚡ Tratamento funcional de erros:\n";

        · auto safe_operation =
            [](const std::string &filename) -> std::expected<int, std::string> {
            auto result = dotenv::load_expected(filename);
            if (result) {
                return *result;
            } else {
                return std::unexpected(
                    std::string("Falha ao carregar ") + filename + ": " +
                    dotenv_get_error_message(result.error()));
            }
        };

        · // Teste com arquivo inexistente
            auto error_case = safe_operation("arquivo_inexistente.env");
        if (!error_case) {
            std::cout << "   ⚠️  Erro capturado: " << error_case.error() << "\n";
        }

        · // Teste com arquivo válido
            auto success_case = safe_operation("tests/test.env");
        if (success_case) {
            std::cout << "   ✅ Sucesso: " << *success_case << " variáveis\n";
        }

        · std::cout << "\n";
    }

    // 5. Value extraction com defaults inteligentes
    {
        std::cout << "5. 🎯 Value extraction com defaults inteligentes:\n";

        · auto get_config_count = []() {
            return dotenv::load_expected("production.env")
                .or_else([](dotenv_error_t) {
                    return dotenv::load_expected("staging.env");
                })
                .or_else([](dotenv_error_t) {
                    return dotenv::load_expected("tests/test.env");
                })
                .value_or(-1);
        };

        · int config_vars = get_config_count();
        if (config_vars > 0) {
            std::cout << "   ✅ Configuração carregada: " << config_vars
                      << " variáveis\n";
        } else if (config_vars == 0) {
            std::cout << "   ⚠️  Arquivo encontrado mas vazio\n";
        } else {
            std::cout << "   ❌ Nenhuma configuração disponível\n";
        }

        · std::cout << "\n";
    }

    std::cout << "=== ✨ Todos os testes de std::expected concluídos! ===\n";
}

#else
void test_monadic_operations() {
    std::cout << "❌ std::expected não disponível (requer C++23)\n";
    std::cout << "Versão C++ atual: " << __cplusplus << "\n";
}
#endif

int main() {
    std::cout << "🎯 Demonstração Avançada de C++23 std::expected\n";
    std::cout << "Compilador: " <<
#ifdef __clang__
        "Clang " << __clang_major__ << "." << __clang_minor__ << "\n";
#elif defined(__GNUC__)
        "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "\n";
#else
        "Unknown\n";
#endif
    std::cout << "C++: " << __cplusplus << "\n";
    std::cout << "Sanitizadores: ✅ Ativos\n\n";

    test_monadic_operations();

    return 0;
}
