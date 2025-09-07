cmake_minimum_required(VERSION 3.15)
project(basic_usage_example
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Basic usage example for dotenv-cpp"
)

# Use the same C++ standard as the main project
# Get the detected C++ standard from parent or use C++20 as fallback
if(DEFINED DOTENV_DETECTED_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD ${DOTENV_DETECTED_CXX_STANDARD})
    message(STATUS "🔧 Using detected C++ standard: ${DOTENV_DETECTED_CXX_STANDARD}")
else()
    set(CMAKE_CXX_STANDARD 20)
    message(STATUS "🔧 Using fallback C++ standard: 20")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Example-specific compiler flags
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    add_compile_options(/W4)
endif()

# Try to find dotenv via find_package first (for installed library)
# But only if we're building standalone (not as part of main project)
if(NOT TARGET dotenv_lib)
    find_package(dotenv QUIET)
endif()

if(TARGET dotenv::dotenv_lib)
    message(STATUS "✅ Using installed dotenv library")
    add_executable(basic_usage_example main.cpp)
    target_link_libraries(basic_usage_example dotenv::dotenv_lib)
elseif(TARGET dotenv_lib)
    message(STATUS "📦 Using local dotenv_lib target from main project")
    add_executable(basic_usage_example main.cpp)
    target_link_libraries(basic_usage_example dotenv_lib)
else()
    message(FATAL_ERROR "❌ No dotenv library found. Please install dotenv or build as part of main project.")
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
