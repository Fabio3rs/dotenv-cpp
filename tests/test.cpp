#include "dotenv.hpp"
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
        dotenv::set("TEST_INT", "");
        dotenv::set("TEST_FLOAT", "");
        dotenv::set("TEST_INVALID", "");
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
