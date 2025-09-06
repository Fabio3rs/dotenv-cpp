#include "dotenv.hpp"
#include <fstream>
#include <gtest/gtest.h>

class DotenvTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Set up environment variables for testing
        dotenv::set("TEST_INT", "123");
        dotenv::set("TEST_FLOAT", "456.78");
        dotenv::set("TEST_INVALID", "invalid");
    }

    void TearDown() override {
        // Clean up environment variables after testing
        dotenv::unset("TEST_INT");
        dotenv::unset("TEST_FLOAT");
        dotenv::unset("TEST_INVALID");
    }
};

TEST_F(DotenvTest, GetInt) {
    int value = dotenv::value_required<int>("TEST_INT");
    EXPECT_EQ(value, 123);
}

TEST_F(DotenvTest, GetFloat) {
    auto value = dotenv::value_required<float>("TEST_FLOAT");
    EXPECT_FLOAT_EQ(value, 456.78F);
}

TEST_F(DotenvTest, GetWithDefault) {
    int value = dotenv::value_or<int>("NON_EXISTENT_KEY", 42);
    EXPECT_EQ(value, 42);
}

TEST_F(DotenvTest, GetInvalidWithFallback) {
    int value = dotenv::value_or<int>("TEST_INVALID", 42);
    EXPECT_EQ(value, 42); // Should fallback to 42 for invalid conversion
}

TEST_F(DotenvTest, GetRequiredThrows) {
    EXPECT_THROW(dotenv::value_required<int>("TEST_INVALID"),
                 std::invalid_argument);
    EXPECT_THROW(dotenv::value_required<int>("NON_EXISTENT_KEY"),
                 std::invalid_argument);
}

TEST_F(DotenvTest, ContainsKey) {
    EXPECT_TRUE(dotenv::contains("TEST_INT"));
    EXPECT_FALSE(dotenv::contains("NON_EXISTENT_KEY"));
}

TEST_F(DotenvTest, SetAndGet) {
    dotenv::set("NEW_KEY", "789");
    std::string value = dotenv::value("NEW_KEY");
    EXPECT_EQ(value, "789");
}

TEST_F(DotenvTest, SaveAndLoad) {
    dotenv::set("SAVE_KEY", "save_value");
    dotenv::save_to_file("test.env");

    dotenv::set("SAVE_KEY", "");
    auto [error, count] = dotenv::load("test.env");
    EXPECT_EQ(error, dotenv::dotenv_error::success);

    std::string value = dotenv::value("SAVE_KEY");
    EXPECT_EQ(value, "save_value");
}

TEST_F(DotenvTest, GetString) {
    dotenv::set("STRING_KEY", "string_value");

    // Teste value() com valor existente
    std::string value = dotenv::value("STRING_KEY");
    EXPECT_EQ(value, "string_value");

    // Teste value() com valor inexistente e default
    std::string default_value = dotenv::value("NONEXISTENT_KEY", "default");
    EXPECT_EQ(default_value, "default");

    // Teste value() com valor inexistente sem default
    std::string empty_value = dotenv::value("NONEXISTENT_KEY2");
    EXPECT_EQ(empty_value, "");
}

TEST_F(DotenvTest, ParserRobust) {
    // Criar arquivo .env de teste com várias features
    std::ofstream env_file("parser_test.env");
    env_file << "# Este é um comentário\n";
    env_file << "\n"; // Linha vazia
    env_file << "SIMPLE_KEY=simple_value\n";
    env_file << "KEY_WITH_SPACES = value_with_spaces \n";
    env_file << "QUOTED_DOUBLE=\"quoted value with spaces\"\n";
    env_file << "QUOTED_SINGLE='single quoted value'\n";
    env_file << "ESCAPED_CHARS=\"line1\\nline2\\ttab\"\n";
    env_file << "VALUE_WITH_EQUALS=key=value=format\n";
    env_file << "   TRIM_KEY   =   trim_value   \n";
    env_file << "# Outro comentário\n";
    env_file << "EMPTY_VALUE=\n";
    env_file << "QUOTED_EMPTY=\"\"\n";
    env_file.close();

    // Carregar o arquivo
    auto [error, count] = dotenv::load("parser_test.env");
    EXPECT_EQ(error, dotenv::dotenv_error::success);
    EXPECT_GT(count, 0);

    // Testar valores carregados
    EXPECT_EQ(dotenv::value("SIMPLE_KEY"), "simple_value");
    EXPECT_EQ(dotenv::value("KEY_WITH_SPACES"), "value_with_spaces");
    EXPECT_EQ(dotenv::value("QUOTED_DOUBLE"), "quoted value with spaces");
    EXPECT_EQ(dotenv::value("QUOTED_SINGLE"), "single quoted value");
    EXPECT_EQ(dotenv::value("ESCAPED_CHARS"), "line1\nline2\ttab");
    EXPECT_EQ(dotenv::value("VALUE_WITH_EQUALS"), "key=value=format");
    EXPECT_EQ(dotenv::value("TRIM_KEY"), "trim_value");
    EXPECT_EQ(dotenv::value("EMPTY_VALUE"), "");
    EXPECT_EQ(dotenv::value("QUOTED_EMPTY"), "");

    // Limpar arquivo de teste
    std::remove("parser_test.env");
}

TEST_F(DotenvTest, OptionalAPI) {
    // Testar try_value com valores existentes
    dotenv::set("OPTIONAL_INT", "42");
    dotenv::set("OPTIONAL_FLOAT", "3.14");
    dotenv::set("OPTIONAL_INVALID", "not_a_number");
    dotenv::set("OPTIONAL_EMPTY", "");

    // Teste try_value com sucesso
    auto int_result = dotenv::try_value<int>("OPTIONAL_INT");
    ASSERT_TRUE(int_result.has_value());
    EXPECT_EQ(int_result.value(), 42);

    auto float_result = dotenv::try_value<float>("OPTIONAL_FLOAT");
    ASSERT_TRUE(float_result.has_value());
    EXPECT_FLOAT_EQ(float_result.value(), 3.14F);

    // Teste try_value com parsing inválido
    auto invalid_result = dotenv::try_value<int>("OPTIONAL_INVALID");
    EXPECT_FALSE(invalid_result.has_value());

    // Teste try_value com chave inexistente
    auto missing_result = dotenv::try_value<int>("NONEXISTENT_KEY");
    EXPECT_FALSE(missing_result.has_value());

    // Teste try_value para strings
    auto string_result = dotenv::try_value("OPTIONAL_INT");
    ASSERT_TRUE(string_result.has_value());
    EXPECT_EQ(string_result.value(), "42");

    // Teste try_value com valor vazio (deve retornar valor vazio, não nullopt)
    auto empty_result = dotenv::try_value("OPTIONAL_EMPTY");
    ASSERT_TRUE(empty_result.has_value());
    EXPECT_EQ(empty_result.value(), "");

    // Teste try_value com chave inexistente
    auto missing_string = dotenv::try_value("NONEXISTENT_KEY");
    EXPECT_FALSE(missing_string.has_value());
}

TEST_F(DotenvTest, CrossPlatformConsistency) {
    // Limpar variável que pode existir de outros testes
    unsetenv("REPLACE_TEST");
    unsetenv("NEW_REPLACE_TEST");

    // Testar semântica consistente de replace entre POSIX e Windows

    // Definir uma variável inicial
    dotenv::set("REPLACE_TEST", "initial_value");
    EXPECT_EQ(dotenv::value("REPLACE_TEST"), "initial_value");

    // Tentar substituir com replace=true (deve sobrescrever)
    dotenv::set("REPLACE_TEST", "replaced_value", dotenv::overwrite::replace);
    EXPECT_EQ(dotenv::value("REPLACE_TEST"), "replaced_value");

    // Tentar definir com replace=false (não deve sobrescrever)
    dotenv::set("REPLACE_TEST", "should_not_replace",
                dotenv::overwrite::preserve);
    EXPECT_EQ(dotenv::value("REPLACE_TEST"), "replaced_value");

    // Testar com chave nova e replace=false (deve definir)
    dotenv::set("NEW_REPLACE_TEST", "new_value", dotenv::overwrite::preserve);
    EXPECT_EQ(dotenv::value("NEW_REPLACE_TEST"), "new_value");
}

TEST_F(DotenvTest, UnsetOperation) {
    // Definir uma variável
    dotenv::set("UNSET_TEST", "test_value");
    EXPECT_TRUE(dotenv::contains("UNSET_TEST"));
    EXPECT_EQ(dotenv::value("UNSET_TEST"), "test_value");

    // Remover a variável
    dotenv::unset("UNSET_TEST");
    EXPECT_FALSE(dotenv::contains("UNSET_TEST"));

    // Verificar que retorna valor padrão após unset
    EXPECT_EQ(dotenv::value("UNSET_TEST", "default"), "default");
}
