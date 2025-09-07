cmake_minimum_required(VERSION 3.15)
project(dotenv_conan_example)

# Find the dotenv package
find_package(dotenv REQUIRED)

# Create executable
add_executable(dotenv_example main.cpp)

# Link dotenv library
target_link_libraries(dotenv_example PRIVATE dotenv::dotenv_lib)

# Use detected C++ standard or fallback
if(DEFINED DOTENV_DETECTED_CXX_STANDARD AND DOTENV_DETECTED_CXX_STANDARD EQUAL 23)
    target_compile_features(dotenv_example PRIVATE cxx_std_23)
    message(STATUS "🔧 Using C++23 for conan example")
else()
    target_compile_features(dotenv_example PRIVATE cxx_std_20)
    message(STATUS "🔧 Using C++20 for conan example")
endif()

# Optional: Add compile flags for better error reporting
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(dotenv_example PRIVATE -Wall -Wextra)
endif()
