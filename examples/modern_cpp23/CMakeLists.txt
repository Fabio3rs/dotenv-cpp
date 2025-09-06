cmake_minimum_required(VERSION 3.15)
project(modern_cpp23_example
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Modern C++23 std::expected example for dotenv-cpp"
)

# Force C++23 for this example
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Check for C++23 support
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
        message(FATAL_ERROR "GCC 12+ required for C++23 features")
    endif()
    add_compile_options(-Wall -Wextra -Wpedantic -fconcepts-diagnostics-depth=3)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
        message(FATAL_ERROR "Clang 15+ required for C++23 features")
    endif()
    add_compile_options(-Wall -Wextra -Wpedantic -stdlib=libc++)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.31")
        message(FATAL_ERROR "MSVC 19.31+ (VS 2022 17.1+) required for C++23 features")
    endif()
    add_compile_options(/W4 /std:c++latest)
endif()

# Force C++23 expected availability
add_compile_definitions(DOTENV_FORCE_CPP23=1)

add_executable(modern_cpp23_example main.cpp)
target_link_libraries(modern_cpp23_example dotenv_lib)

# Copy configuration files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app.env ${CMAKE_CURRENT_BINARY_DIR}/app.env COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fallback.env ${CMAKE_CURRENT_BINARY_DIR}/fallback.env COPYONLY)

set_target_properties(modern_cpp23_example PROPERTIES
    OUTPUT_NAME "modern_cpp23"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

message(STATUS "⚡ Modern C++23 example configured with std::expected")
