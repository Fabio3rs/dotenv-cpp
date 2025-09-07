cmake_minimum_required(VERSION 3.15)
project(modern_cpp20_example
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Modern C++20 features example for dotenv-cpp"
)

# Force C++20 for this example
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enhanced compiler flags for C++20 features
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -fconcepts-diagnostics-depth=3)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    add_compile_options(/W4 /std:c++20)
endif()

# Enable verbose feature detection
add_compile_definitions(DOTENV_VERBOSE_FEATURES=1)

# Try to find dotenv via find_package first (for installed library)
# But only if we're building standalone (not as part of main project)
if(NOT TARGET dotenv_lib)
    find_package(dotenv QUIET)
endif()

add_executable(modern_cpp20_example main.cpp)

if(TARGET dotenv::dotenv_lib)
    target_link_libraries(modern_cpp20_example dotenv::dotenv_lib)
elseif(TARGET dotenv_lib)
    target_link_libraries(modern_cpp20_example dotenv_lib)
else()
    message(FATAL_ERROR "❌ No dotenv library found")
endif()

# Copy configuration files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.env ${CMAKE_CURRENT_BINARY_DIR}/config.env COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/production.env ${CMAKE_CURRENT_BINARY_DIR}/production.env COPYONLY)

set_target_properties(modern_cpp20_example PROPERTIES
    OUTPUT_NAME "modern_cpp20"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

message(STATUS "🚀 Modern C++20 example configured with enhanced features")
