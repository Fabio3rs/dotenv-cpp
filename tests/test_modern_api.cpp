#include "dotenv.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

class ModernDotenvAPITest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Create test directory and files
        // Garante diretório único por teste usando PID
        auto pid = static_cast<unsigned long>(::getpid());
        test_dir = std::filesystem::temp_directory_path() /
                   ("dotenv_modern_test_" + std::to_string(pid));
        std::filesystem::create_directories(test_dir);

        // Create test .env file
        test_env_file = test_dir / "test.env";
        create_test_env_file();

        // Create empty .env file for testing
        empty_env_file = test_dir / "empty.env";
        std::ofstream empty_file(empty_env_file); // Create empty file
        empty_file.close();

        // Create invalid .env file
        invalid_env_file = test_dir / "invalid.env";
        create_invalid_env_file();
    }

    void TearDown() override {
        // Clean up test files
        std::error_code ec;
        std::filesystem::remove_all(test_dir, ec);
    }

    void create_test_env_file() {
        std::ofstream file(test_env_file);
        file << "# Test configuration\n";
        file << "APP_NAME=TestApp\n";
        file << "DEBUG_MODE=true\n";
        file << "PORT=8080\n";
        file << "TIMEOUT=30.5\n";
        file << "MAX_CONNECTIONS=100\n";
        file << "EMPTY_VALUE=\n";
        file << "QUOTED_VALUE=\"hello world\"\n";
        file << "BOOLEAN_TRUE=yes\n";
        file << "BOOLEAN_FALSE=no\n";
    }

    void create_invalid_env_file() {
        std::ofstream file(invalid_env_file);
        file << "INVALID_NUMBER=not_a_number\n";
        file << "MALFORMED_LINE_WITHOUT_EQUALS\n";
    }

    std::filesystem::path test_dir;
    std::filesystem::path test_env_file;
    std::filesystem::path empty_env_file;
    std::filesystem::path invalid_env_file;
};

// ==== Test Modern Load API ====

TEST_F(ModernDotenvAPITest, LoadWithDefaultOptions) {
    dotenv::load_options opts;
    auto [error, result] = dotenv::load(test_env_file.string(), opts);

    EXPECT_EQ(error, dotenv::dotenv_error::success);
    EXPECT_GT(result, 0);
    EXPECT_EQ(dotenv::value("APP_NAME"), "TestApp");
    EXPECT_EQ(dotenv::get("DEBUG_MODE"), "true");
}

TEST_F(ModernDotenvAPITest, LoadWithCustomOptions) {
    dotenv::load_options opts{.overwrite_policy = dotenv::overwrite::preserve,
                              .apply_to_process = dotenv::process_env_apply::no,
                              .backend = dotenv::parse_backend::traditional};

    // Set environment variable that should be preserved
    setenv("APP_NAME", "ExistingApp", 1);

    auto [error, result] = dotenv::load(test_env_file.string(), opts);

    EXPECT_EQ(error, dotenv::dotenv_error::success);
    EXPECT_GT(result, 0);
    EXPECT_EQ(dotenv::value("APP_NAME"), "TestApp"); // From internal storage
    EXPECT_EQ(std::string(getenv("APP_NAME") ?: ""),
              "ExistingApp"); // Process env preserved
}

#if __cplusplus >= 202302L
TEST_F(ModernDotenvAPITest, LoadExpected) {
    auto result = dotenv::load_expected(test_env_file.string());

    ASSERT_TRUE(result.has_value());
    EXPECT_GT(*result, 0);
}

TEST_F(ModernDotenvAPITest, LoadExpectedFileNotFound) {
    auto result = dotenv::load_expected("nonexistent.env");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), DOTENV_ERROR_FILE_NOT_FOUND);
}

TEST_F(ModernDotenvAPITest, LoadExpectedMonadicOperations) {
    auto result = dotenv::load_expected(test_env_file.string())
                      .transform([](int count) {
                          return count * 2; // Double the count
                      })
                      .or_else([](auto) {
                          return std::expected<int, dotenv_error_t>(0);
                      });

    ASSERT_TRUE(result.has_value());
    EXPECT_GT(*result, 0);
}
#endif

// ==== Test Modern Variable Access API ====

TEST_F(ModernDotenvAPITest, ValueAccess) {
    dotenv::load(test_env_file.string());

    EXPECT_EQ(dotenv::value("APP_NAME"), "TestApp");
    EXPECT_EQ(dotenv::value("NONEXISTENT", "default"), "default");
    EXPECT_EQ(dotenv::get("APP_NAME"), "TestApp");
}

TEST_F(ModernDotenvAPITest, GetOrNumericTypes) {
    dotenv::load(test_env_file.string());

    EXPECT_EQ(dotenv::value_or<int>("PORT", 0), 8080);
    EXPECT_EQ(dotenv::value_or<double>("TIMEOUT", 0.0), 30.5);
    EXPECT_EQ(dotenv::value_or<int>("NONEXISTENT", 42), 42);
    EXPECT_EQ(dotenv::value_or<int>("INVALID_NUMBER", 99),
              99); // Fallback on invalid
}

TEST_F(ModernDotenvAPITest, GetRequired) {
    dotenv::load(test_env_file.string());

    EXPECT_EQ(dotenv::value_required<int>("PORT"), 8080);
    EXPECT_EQ(dotenv::value_required<double>("TIMEOUT"), 30.5);

    EXPECT_THROW(dotenv::value_required<int>("NONEXISTENT"),
                 std::invalid_argument);
}

TEST_F(ModernDotenvAPITest, TryGet) {
    dotenv::load(test_env_file.string());

    auto port = dotenv::try_value<int>("PORT");
    ASSERT_TRUE(port.has_value());
    EXPECT_EQ(*port, 8080);

    auto nonexistent = dotenv::try_value<int>("NONEXISTENT");
    EXPECT_FALSE(nonexistent.has_value());

    // Load invalid file and test parsing failure
    dotenv::load(invalid_env_file.string());
    auto invalid = dotenv::try_value<int>("INVALID_NUMBER");
    EXPECT_FALSE(invalid.has_value());
}

TEST_F(ModernDotenvAPITest, TryValue) {
    dotenv::load(test_env_file.string());

    auto app_name = dotenv::try_value("APP_NAME");
    ASSERT_TRUE(app_name.has_value());
    EXPECT_EQ(*app_name, "TestApp");

    auto nonexistent = dotenv::try_value("NONEXISTENT");
    EXPECT_FALSE(nonexistent.has_value());
}

// ==== Test Type-Safe Operations ====

TEST_F(ModernDotenvAPITest, TypeSafeSet) {
    dotenv::set("TEST_VAR", "value1", dotenv::overwrite::replace);
    EXPECT_EQ(dotenv::value("TEST_VAR"), "value1");

    dotenv::set("TEST_VAR", "value2", dotenv::overwrite::preserve);
    EXPECT_EQ(dotenv::value("TEST_VAR"), "value1"); // Should be preserved

    dotenv::set("TEST_VAR", "value3", dotenv::overwrite::replace);
    EXPECT_EQ(dotenv::value("TEST_VAR"), "value3"); // Should be replaced
}

TEST_F(ModernDotenvAPITest, ApplyInternalToProcessEnv) {
    // Clear any existing APP_NAME variable first
    unsetenv("APP_NAME");

    // Load without applying to process env
    dotenv::load_options opts{.apply_to_process =
                                  dotenv::process_env_apply::no};
    dotenv::load(test_env_file.string(), opts);

    // Variable should be in internal storage but not process env
    EXPECT_EQ(dotenv::value("APP_NAME"), "TestApp");
    EXPECT_EQ(getenv("APP_NAME"), nullptr);

    // Apply to process env
    dotenv::apply_internal_to_process_env(dotenv::overwrite::replace);

    // Now should be in both
    EXPECT_EQ(dotenv::value("APP_NAME"), "TestApp");
    EXPECT_EQ(std::string(getenv("APP_NAME") ?: ""), "TestApp");
}

// ==== Test Backend Selection ====

TEST_F(ModernDotenvAPITest, ForceTraditionalBackend) {
    dotenv::load_options opts{.backend = dotenv::parse_backend::traditional};

    auto [error, result] =
        dotenv::load_traditional(test_env_file.string(), opts);
    EXPECT_EQ(error, dotenv::dotenv_error::success);
    EXPECT_GT(result, 0);
    EXPECT_EQ(dotenv::value("APP_NAME"), "TestApp");
}

#ifdef DOTENV_SIMD_ENABLED
TEST_F(ModernDotenvAPITest, ForceSIMDBackend) {
    dotenv::load_options opts{.backend = dotenv::parse_backend::simd};

    auto [error, result] = dotenv::load_simd(test_env_file.string(), opts);
    EXPECT_EQ(error, dotenv::dotenv_error::success);
    EXPECT_GT(result, 0);
    EXPECT_EQ(dotenv::value("APP_NAME"), "TestApp");
}
#endif

// ==== Test File Operations ====

TEST_F(ModernDotenvAPITest, SaveToFile) {
    dotenv::load(test_env_file.string());

    auto output_file = test_dir / "output.env";
    dotenv::save_to_file(output_file.string());

    EXPECT_TRUE(std::filesystem::exists(output_file));

    // Verify saved content by loading it back
    dotenv::load(output_file.string());
    EXPECT_EQ(dotenv::value("APP_NAME"), "TestApp");
}

// ==== Test Deprecated API Warnings ====

TEST_F(ModernDotenvAPITest, DeprecatedAPIStillWorks) {
    // These should work but generate deprecation warnings during compilation

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

    // Test legacy load with 3 parameters
    auto [error, result] =
        dotenv::load_with_status(test_env_file.string(), 1, true);
    EXPECT_EQ(error, DOTENV_SUCCESS);
    EXPECT_GT(result, 0);

    // Test the value function which should work
    std::string old_api_value{dotenv::get("APP_NAME")};
    EXPECT_EQ(old_api_value, "TestApp");

    // Test try_value which is the modern way
    auto modern_optional = dotenv::try_value("APP_NAME");
    ASSERT_TRUE(modern_optional.has_value());
    EXPECT_EQ(*modern_optional, "TestApp");

#pragma clang diagnostic pop
}
