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
    int value = dotenv::get<int>("TEST_INT");
    EXPECT_EQ(value, 123);
}

TEST_F(DotenvTest, GetFloat) {
    float value = dotenv::get<float>("TEST_FLOAT");
    EXPECT_FLOAT_EQ(value, 456.78f);
}

TEST_F(DotenvTest, GetWithDefault) {
    int value = dotenv::get<int>("NON_EXISTENT_KEY", 42);
    EXPECT_EQ(value, 42);
}

TEST_F(DotenvTest, GetWithNoThrowPolicy) {
    int value = dotenv::get<int>(dotenv::NoThrowPolicy{}, "TEST_INVALID", 42);
    EXPECT_EQ(value, 42);
}

TEST_F(DotenvTest, GetWithThrowPolicy) {
    EXPECT_THROW(dotenv::get<int>(dotenv::ThrowPolicy{}, "TEST_INVALID"),
                 std::invalid_argument);
}

TEST_F(DotenvTest, HasKey) {
    EXPECT_TRUE(dotenv::has("TEST_INT"));
    EXPECT_FALSE(dotenv::has("NON_EXISTENT_KEY"));
}

TEST_F(DotenvTest, SetAndGet) {
    dotenv::set("NEW_KEY", "789");
    std::string value(dotenv::get("NEW_KEY"));
    EXPECT_EQ(value, "789");
}

TEST_F(DotenvTest, SaveAndLoad) {
    dotenv::set("SAVE_KEY", "save_value");
    dotenv::save("test.env");

    dotenv::set("SAVE_KEY", "");
    dotenv::load("test.env");

    std::string value(dotenv::get("SAVE_KEY"));
    EXPECT_EQ(value, "save_value");
}

TEST_F(DotenvTest, GetString) {
    dotenv::set("STRING_KEY", "string_value");

    // Teste get_string com valor existente
    std::string value = dotenv::get_string("STRING_KEY");
    EXPECT_EQ(value, "string_value");

    // Teste get_string com valor inexistente e default
    std::string default_value =
        dotenv::get_string("NONEXISTENT_KEY", "default");
    EXPECT_EQ(default_value, "default");

    // Teste get_string com valor inexistente sem default
    std::string empty_value = dotenv::get_string("NONEXISTENT_KEY2");
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
    int count = dotenv::load("parser_test.env");
    EXPECT_GT(count, 0);

    // Testar valores carregados
    EXPECT_EQ(dotenv::get_string("SIMPLE_KEY"), "simple_value");
    EXPECT_EQ(dotenv::get_string("KEY_WITH_SPACES"), "value_with_spaces");
    EXPECT_EQ(dotenv::get_string("QUOTED_DOUBLE"), "quoted value with spaces");
    EXPECT_EQ(dotenv::get_string("QUOTED_SINGLE"), "single quoted value");
    EXPECT_EQ(dotenv::get_string("ESCAPED_CHARS"), "line1\nline2\ttab");
    EXPECT_EQ(dotenv::get_string("VALUE_WITH_EQUALS"), "key=value=format");
    EXPECT_EQ(dotenv::get_string("TRIM_KEY"), "trim_value");
    EXPECT_EQ(dotenv::get_string("EMPTY_VALUE"), "");
    EXPECT_EQ(dotenv::get_string("QUOTED_EMPTY"), "");

    // Limpar arquivo de teste
    std::remove("parser_test.env");
}

TEST_F(DotenvTest, OptionalAPI) {
    // Testar get_optional com valores existentes
    dotenv::set("OPTIONAL_INT", "42");
    dotenv::set("OPTIONAL_FLOAT", "3.14");
    dotenv::set("OPTIONAL_INVALID", "not_a_number");
    dotenv::set("OPTIONAL_EMPTY", "");

    // Teste get_optional com sucesso
    auto int_result = dotenv::get_optional<int>("OPTIONAL_INT");
    ASSERT_TRUE(int_result.has_value());
    EXPECT_EQ(int_result.value(), 42);

    auto float_result = dotenv::get_optional<float>("OPTIONAL_FLOAT");
    ASSERT_TRUE(float_result.has_value());
    EXPECT_FLOAT_EQ(float_result.value(), 3.14f);

    // Teste get_optional com parsing inválido
    auto invalid_result = dotenv::get_optional<int>("OPTIONAL_INVALID");
    EXPECT_FALSE(invalid_result.has_value());

    // Teste get_optional com chave inexistente
    auto missing_result = dotenv::get_optional<int>("NONEXISTENT_KEY");
    EXPECT_FALSE(missing_result.has_value());

    // Teste get_optional_string
    auto string_result = dotenv::get_optional_string("OPTIONAL_INT");
    ASSERT_TRUE(string_result.has_value());
    EXPECT_EQ(string_result.value(), "42");

    // Teste get_optional_string com valor vazio (deve retornar valor vazio, não
    // nullopt)
    auto empty_result = dotenv::get_optional_string("OPTIONAL_EMPTY");
    ASSERT_TRUE(empty_result.has_value());
    EXPECT_EQ(empty_result.value(), "");

    // Teste get_optional_string com chave inexistente
    auto missing_string = dotenv::get_optional_string("NONEXISTENT_KEY");
    EXPECT_FALSE(missing_string.has_value());
}

TEST_F(DotenvTest, CrossPlatformConsistency) {
    // Limpar variável que pode existir de outros testes
    unsetenv("REPLACE_TEST");
    unsetenv("NEW_REPLACE_TEST");

    // Testar semântica consistente de replace entre POSIX e Windows

    // Definir uma variável inicial
    dotenv::set("REPLACE_TEST", "initial_value");
    EXPECT_EQ(dotenv::get_string("REPLACE_TEST"), "initial_value");

    // Tentar substituir com replace=true (deve sobrescrever)
    dotenv::set("REPLACE_TEST", "replaced_value", true);
    EXPECT_EQ(dotenv::get_string("REPLACE_TEST"), "replaced_value");

    // Tentar definir com replace=false (não deve sobrescrever)
    dotenv::set("REPLACE_TEST", "should_not_replace", false);
    EXPECT_EQ(dotenv::get_string("REPLACE_TEST"), "replaced_value");

    // Testar com chave nova e replace=false (deve definir)
    dotenv::set("NEW_REPLACE_TEST", "new_value", false);
    EXPECT_EQ(dotenv::get_string("NEW_REPLACE_TEST"), "new_value");
}

TEST_F(DotenvTest, UnsetOperation) {
    // Definir uma variável
    dotenv::set("UNSET_TEST", "test_value");
    EXPECT_TRUE(dotenv::has("UNSET_TEST"));
    EXPECT_EQ(dotenv::get_string("UNSET_TEST"), "test_value");

    // Remover a variável
    dotenv::unset("UNSET_TEST");
    EXPECT_FALSE(dotenv::has("UNSET_TEST"));

    // Verificar que retorna valor padrão após unset
    EXPECT_EQ(dotenv::get_string("UNSET_TEST", "default"), "default");
}
