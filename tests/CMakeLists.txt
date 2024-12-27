add_executable(dotenv_tests test.cpp)
target_link_libraries(dotenv_tests dotenv_lib GTest::GTest GTest::Main)
gtest_discover_tests(dotenv_tests)
