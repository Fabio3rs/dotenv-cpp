set(TEST_SOURCES
    test.cpp
    test_modern_api.cpp
)

# Add SIMD tests if enabled
if(DOTENV_ENABLE_SIMD)
    list(APPEND TEST_SOURCES test_simd.cpp)
endif()

# Add standalone test executables for demonstration
add_executable(test_expected test_expected.cpp)
target_link_libraries(test_expected dotenv_lib)
target_compile_features(test_expected PRIVATE cxx_std_20)

add_executable(test_expected_advanced test_expected_advanced.cpp)
target_link_libraries(test_expected_advanced dotenv_lib)
target_compile_features(test_expected_advanced PRIVATE cxx_std_20)

add_executable(test_process_env test_process_env.cpp)
target_link_libraries(test_process_env dotenv_lib)
target_compile_features(test_process_env PRIVATE cxx_std_20)

# Main test suite with gtest
add_executable(dotenv_tests ${TEST_SOURCES})
target_link_libraries(dotenv_tests dotenv_lib GTest::GTest GTest::Main)
target_compile_features(dotenv_tests PRIVATE cxx_std_20)
gtest_discover_tests(dotenv_tests)
