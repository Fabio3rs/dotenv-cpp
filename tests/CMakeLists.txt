set(TEST_SOURCES test.cpp)

# Add SIMD tests if enabled
if(DOTENV_ENABLE_SIMD)
    list(APPEND TEST_SOURCES test_simd.cpp)
endif()

add_executable(dotenv_tests ${TEST_SOURCES})
target_link_libraries(dotenv_tests dotenv_lib GTest::GTest GTest::Main)
target_compile_features(dotenv_tests PRIVATE cxx_std_20)
gtest_discover_tests(dotenv_tests)
