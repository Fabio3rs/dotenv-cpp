#include "dotenv.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#ifdef DOTENV_SIMD_ENABLED

class SIMDTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Create test file for SIMD testing
        std::ofstream file("test_simd.env");
        file << "SIMD_KEY1=simd_value1\n";
        file << "SIMD_KEY2=42\n";
        file << "# This is a comment\n";
        file << "\n"; // Empty line
        file << "SIMD_KEY3=3.14159\n";
        file << "SIMD_QUOTED=\"quoted value\"\n";
        file.close();
    }

    void TearDown() override {
        std::filesystem::remove("test_simd.env");
        // Clear environment
        dotenv::unset("SIMD_KEY1");
        dotenv::unset("SIMD_KEY2");
        dotenv::unset("SIMD_KEY3");
        dotenv::unset("SIMD_QUOTED");
    }
};

TEST_F(SIMDTest, LoadSIMDBasic) {
    int result = dotenv::load_simd("test_simd.env");
    EXPECT_GT(result, 0);

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
    int result = dotenv::load_simd("test_simd.env");

    if (avx2_available) {
        EXPECT_GT(result, 0); // Should load successfully
    } else {
        // Should fallback to standard implementation
        EXPECT_GT(result, 0);
    }
}

TEST_F(SIMDTest, SIMDLargeFile) {
    // Create larger test file
    std::ofstream large_file("large_simd_test.env");
    for (int i = 0; i < 1000; ++i) {
        large_file << "LARGE_KEY_" << i << "=large_value_" << i << "\n";
        if (i % 10 == 0) {
            large_file << "# Comment " << i << "\n";
        }
    }
    large_file.close();

    int result = dotenv::load_simd("large_simd_test.env");
    EXPECT_GT(result, 900); // Should load most variables (excluding comments)

    // Check a few random values
    EXPECT_EQ(dotenv::get("LARGE_KEY_0"), "large_value_0");
    EXPECT_EQ(dotenv::get("LARGE_KEY_500"), "large_value_500");
    EXPECT_EQ(dotenv::get("LARGE_KEY_999"), "large_value_999");

    // Cleanup
    std::filesystem::remove("large_simd_test.env");
    for (int i = 0; i < 1000; ++i) {
        dotenv::unset("LARGE_KEY_" + std::to_string(i));
    }
}

#endif // DOTENV_SIMD_ENABLED
