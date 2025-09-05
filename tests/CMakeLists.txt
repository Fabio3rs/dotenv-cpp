add_executable(dotenv_tests test.cpp)
target_link_libraries(dotenv_tests dotenv_lib GTest::GTest GTest::Main)
target_compile_features(dotenv_tests PRIVATE cxx_std_20)
gtest_discover_tests(dotenv_tests)
