# Examples directory builds individual examples as standalone executables
# Each example demonstrates different usage patterns and integration approaches

cmake_minimum_required(VERSION 3.20)

# Allow examples to be built independently or as part of main project
if(NOT TARGET dotenv::dotenv_lib)
    message(STATUS "🎯 Building examples for dotenv-cpp ${PROJECT_VERSION}")

    # Use the installed library if building independently
    find_package(dotenv REQUIRED)
    message(STATUS "✅ Using installed dotenv library")
else()
    message(STATUS "🎯 Building examples for dotenv-cpp ${PROJECT_VERSION}")
    message(STATUS "✅ Using local dotenv library from source tree")
endif()

# Add examples
add_subdirectory(basic_usage)
add_subdirectory(cmake_integration)
add_subdirectory(conan_usage)
add_subdirectory(modern_cpp20)

# Add these when implemented:
# add_subdirectory(modern_cpp23)
# add_subdirectory(performance_simd)
# add_subdirectory(error_handling)
# add_subdirectory(cross_platform)
# add_subdirectory(advanced_usage)

message(STATUS "✅ Examples configured successfully")
