#include "dotenv.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <thread>
#include <unistd.h>

#ifdef DOTENV_SIMD_ENABLED

class SIMDTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Create unique test directory per process to avoid conflicts
        auto pid = static_cast<unsigned long>(::getpid());
        auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
        test_dir = std::filesystem::temp_directory_path() /
                   ("dotenv_simd_test_" + std::to_string(pid) + "_" +
                    std::to_string(tid));
        std::filesystem::create_directories(test_dir);

        // Create test file paths
        test_simd_file = test_dir / "test_simd.env";
        large_simd_file = test_dir / "large_simd_test.env";

        // Create test .env file
        std::ofstream env_file(test_simd_file);
        env_file << "SIMD_KEY1=simd_value1\n";
        env_file << "SIMD_KEY2=42\n";
        env_file << "SIMD_KEY3=3.14159\n";
        env_file.close();
    }

    void TearDown() override {
        // Clean up environment variables
        dotenv::unset("SIMD_KEY1");
        dotenv::unset("SIMD_KEY2");
        dotenv::unset("SIMD_KEY3");

        // Clean up test files
        std::error_code error_code;
        std::filesystem::remove_all(test_dir, error_code);
    }

    std::filesystem::path test_dir;
    std::filesystem::path test_simd_file;
    std::filesystem::path large_simd_file;
};

TEST_F(SIMDTest, LoadSIMDBasic) {
    auto [error, count] = dotenv::load_simd_legacy(test_simd_file.string());
    EXPECT_EQ(error, dotenv::dotenv_error::success);
    EXPECT_GT(count, 0);

    EXPECT_EQ(dotenv::get("SIMD_KEY1"), "simd_value1");
    EXPECT_EQ(dotenv::get("SIMD_KEY2"), "42");
    EXPECT_EQ(dotenv::get("SIMD_KEY3"), "3.14159");
}

TEST_F(SIMDTest, LoadSIMDWithFallback) {
    // Test that SIMD loads same data as standard load
    dotenv::load(test_simd_file.string());
    std::string standard_val1 = std::string(dotenv::get("SIMD_KEY1"));
    std::string standard_val2 = std::string(dotenv::get("SIMD_KEY2"));

    // Clear and load with SIMD
    dotenv::unset("SIMD_KEY1");
    dotenv::unset("SIMD_KEY2");

    dotenv::load_simd(test_simd_file.string());
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
    auto [error, count] = dotenv::load_simd_legacy(test_simd_file.string());

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
    std::ofstream large_file(large_simd_file);
    constexpr int large_file_size = 1000;
    constexpr int comment_frequency = 10;
    for (int i = 0; i < large_file_size; ++i) {
        large_file << "LARGE_KEY_" << i << "=large_value_" << i << "\n";
        if (i % comment_frequency == 0) {
            large_file << "# Comment " << i << "\n";
        }
    }
    large_file.close();

    auto [error, count] = dotenv::load_simd_legacy(large_simd_file.string());
    EXPECT_EQ(error, dotenv::dotenv_error::success);
    EXPECT_GT(count, 900); // Should load most variables (excluding comments)

    // Check a few random values
    EXPECT_EQ(dotenv::get("LARGE_KEY_0"), "large_value_0");
    EXPECT_EQ(dotenv::get("LARGE_KEY_500"), "large_value_500");
    EXPECT_EQ(dotenv::get("LARGE_KEY_999"), "large_value_999");

    // Cleanup variables loaded from file
    for (int i = 0; i < large_file_size; ++i) {
        dotenv::unset("LARGE_KEY_" + std::to_string(i));
    }
}

#endif // DOTENV_SIMD_ENABLED
