# Examples directory builds individual examples as standalone executables
# Each example demonstrates different usage patterns and integration approaches

cmake_minimum_required(VERSION 3.20)

# Allow examples to be built independently or as part of main project
# Check if we're building as part of the main project by looking for the dotenv_lib target
if(NOT TARGET dotenv_lib)
    message(STATUS "🎯 Building examples standalone for dotenv-cpp ${PROJECT_VERSION}")

    # Use the installed library if building independently
    find_package(dotenv REQUIRED)
    message(STATUS "✅ Using installed dotenv library")

    # Set a variable to indicate we're using the installed version
    set(DOTENV_USE_INSTALLED TRUE)
else()
    message(STATUS "🎯 Building examples for dotenv-cpp ${PROJECT_VERSION}")
    message(STATUS "✅ Using local dotenv library from source tree")

    # Create alias for compatibility when building from source (only if it doesn't exist)
    if(NOT TARGET dotenv::dotenv_lib)
        add_library(dotenv::dotenv_lib ALIAS dotenv_lib)
    endif()
    set(DOTENV_USE_INSTALLED FALSE)
endif()

# Add examples
add_subdirectory(basic_usage)
add_subdirectory(cmake_integration)
add_subdirectory(conan_usage)
add_subdirectory(modern_cpp20)
add_subdirectory(modern_cpp23)

# Add these when implemented:
# add_subdirectory(performance_simd)
# add_subdirectory(error_handling)
# add_subdirectory(cross_platform)
# add_subdirectory(advanced_usage)

message(STATUS "✅ Examples configured successfully")
