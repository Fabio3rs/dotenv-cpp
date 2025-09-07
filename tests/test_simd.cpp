#include "dotenv.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#ifdef DOTENV_SIMD_ENABLED

class SIMDTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Create test .env file
        std::ofstream env_file("test_simd.env");
        env_file << "SIMD_KEY1=simd_value1\n";
        env_file << "SIMD_KEY2=42\n";
        env_file << "SIMD_KEY3=3.14159\n";
        env_file.close();
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove("test_simd.env");
        std::filesystem::remove("large_simd_test.env");
    }
};

TEST_F(SIMDTest, LoadSIMDBasic) {
    auto [error, count] = dotenv::load_simd_legacy("test_simd.env");
    EXPECT_EQ(error, dotenv::dotenv_error::success);
    EXPECT_GT(count, 0);

    EXPECT_EQ(dotenv::get("SIMD_KEY1"), "simd_value1");
    EXPECT_EQ(dotenv::get("SIMD_KEY2"), "42");
    EXPECT_EQ(dotenv::get("SIMD_KEY3"), "3.14159");
}

TEST_F(SIMDTest, LoadSIMDWithFallback) {
    // Test that SIMD loads same data as standard load
    dotenv::load("test_simd.env");
    std::string standard_val1 = std::string(dotenv::get("SIMD_KEY1"));
    std::string standard_val2 = std::string(dotenv::get("SIMD_KEY2"));

    // Clear and load with SIMD
    dotenv::unset("SIMD_KEY1");
    dotenv::unset("SIMD_KEY2");

    dotenv::load_simd("test_simd.env");
    std::string simd_val1 = std::string(dotenv::get("SIMD_KEY1"));
    std::string simd_val2 = std::string(dotenv::get("SIMD_KEY2"));

    EXPECT_EQ(standard_val1, simd_val1);
    EXPECT_EQ(standard_val2, simd_val2);
}

TEST_F(SIMDTest, SIMDAvailabilityCheck) {
    // This test will pass regardless of AVX2 availability
    // but will validate the detection logic
    bool avx2_available = dotenv::simd::is_avx2_available();

    // Should not crash
    auto [error, count] = dotenv::load_simd_legacy("test_simd.env");

    if (avx2_available) {
        EXPECT_EQ(error, dotenv::dotenv_error::success);
        EXPECT_GT(count, 0); // Should load successfully
    } else {
        // Should fallback to standard implementation
        EXPECT_EQ(error, dotenv::dotenv_error::success);
        EXPECT_GT(count, 0);
    }
}

TEST_F(SIMDTest, SIMDLargeFile) {
    // Create larger test file
    std::ofstream large_file("large_simd_test.env");
    constexpr int large_file_size = 1000;
    constexpr int comment_frequency = 10;
    for (int i = 0; i < large_file_size; ++i) {
        large_file << "LARGE_KEY_" << i << "=large_value_" << i << "\n";
        if (i % comment_frequency == 0) {
            large_file << "# Comment " << i << "\n";
        }
    }
    large_file.close();

    auto [error, count] = dotenv::load_simd_legacy("large_simd_test.env");
    EXPECT_EQ(error, dotenv::dotenv_error::success);
    EXPECT_GT(count, 900); // Should load most variables (excluding comments)

    // Check a few random values
    EXPECT_EQ(dotenv::get("LARGE_KEY_0"), "large_value_0");
    EXPECT_EQ(dotenv::get("LARGE_KEY_500"), "large_value_500");
    EXPECT_EQ(dotenv::get("LARGE_KEY_999"), "large_value_999");

    // Cleanup
    std::filesystem::remove("large_simd_test.env");
    for (int i = 0; i < large_file_size; ++i) {
        dotenv::unset("LARGE_KEY_" + std::to_string(i));
    }
}

#endif // DOTENV_SIMD_ENABLED
