cmake_minimum_required(VERSION 3.15)
project(basic_usage_example
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Basic usage example for dotenv-cpp"
)

# Modern C++20 as minimum requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Example-specific compiler flags
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    add_compile_options(/W4)
endif()

# Try to find dotenv via find_package first (for installed library)
find_package(dotenv QUIET)

if(dotenv_FOUND)
    message(STATUS "✅ Using installed dotenv library")
    add_executable(basic_usage_example main.cpp)
    target_link_libraries(basic_usage_example dotenv::dotenv_lib)
else()
    message(STATUS "📦 dotenv not found, using local build")
    # Fallback: use parent project's dotenv_lib target
    add_executable(basic_usage_example main.cpp)
    target_link_libraries(basic_usage_example dotenv_lib)
endif()

# Copy example.env to build directory for runtime
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/example.env
    ${CMAKE_CURRENT_BINARY_DIR}/example.env
    COPYONLY
)

# Set runtime properties
set_target_properties(basic_usage_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT_NAME "basic_usage"
)

message(STATUS "🎯 Basic usage example configured")
