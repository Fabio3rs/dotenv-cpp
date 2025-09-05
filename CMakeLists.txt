cmake_minimum_required(VERSION 3.12)

# Versioning
set(DOTENV_VERSION_MAJOR 2)
set(DOTENV_VERSION_MINOR 0)
set(DOTENV_VERSION_PATCH 0)
set(DOTENV_VERSION "${DOTENV_VERSION_MAJOR}.${DOTENV_VERSION_MINOR}.${DOTENV_VERSION_PATCH}")

project(dotenv
    VERSION ${DOTENV_VERSION}
    LANGUAGES C CXX
    DESCRIPTION "Modern C++20 .env file parser with thread safety and Unicode support"
    HOMEPAGE_URL "https://github.com/Fabio3rs/dotenv-cpp"
)

set(IS_CONAN FALSE)
get_cmake_property(all_variables CACHE_VARIABLES)
foreach (var ${all_variables})
    if ("${var}" MATCHES "CONAN")
        set(IS_CONAN TRUE)
    endif()
endforeach()

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}" AND NOT IS_CONAN)
    add_compile_definitions(-DFORTIFY_SOURCE=3)
    message(STATUS "Setting up dotenv project")
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_C_EXTENSIONS OFF)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        message(STATUS "Setting G++ flags")
        add_compile_options(-Wall -Wextra -Wformat-security -Wconversion -Wsign-conversion -Wno-gnu -Wno-gnu-statement-expression)
        # -Werror opcional para CI
        if(DOTENV_ENABLE_WERROR)
            add_compile_options(-Werror)
        endif()
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        message(STATUS "Setting MSVC flags")
        add_compile_options(/W4)
        # /WX opcional para CI
        if(DOTENV_ENABLE_WERROR)
            add_compile_options(/WX)
        endif()
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message(STATUS "Setting Clang flags")
        add_compile_options(-Wall -Wextra -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-global-constructors -Wno-exit-time-destructors -Wno-weak-vtables -Wno-documentation -Wno-documentation-unknown-command)
        # -Werror opcional para CI
        if(DOTENV_ENABLE_WERROR)
            add_compile_options(-Werror)
        endif()
    endif()

    option(DOTENV_ENABLE_TESTS "Enable tests" ON)
    option(DOTENV_ENABLE_SANITIZERS "Enable sanitizers" ON)
    option(DOTENV_BUILD_EXE "Enable a simple dotenv test executable" ON)
    option(DOTENV_ENABLE_BENCHMARKS "Enable benchmarks with Google Benchmark" OFF)
    option(DOTENV_ENABLE_WERROR "Enable -Werror/-WX (recommended for CI)" OFF)
else()
    option(DOTENV_ENABLE_TESTS "Enable tests" OFF)
    option(DOTENV_ENABLE_SANITIZERS "Enable sanitizers" OFF)
    option(DOTENV_BUILD_EXE "Enable a simple dotenv test executable" OFF)
    option(DOTENV_ENABLE_BENCHMARKS "Enable benchmarks with Google Benchmark" OFF)
endif()

if (IS_CONAN)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_EXTENSIONS OFF)
endif()

if(DOTENV_ENABLE_TESTS)
    include(CTest)
    enable_testing()
    find_package(GTest)

    if(GTest_FOUND)
        include(GoogleTest)
    endif()
endif()

if(DOTENV_ENABLE_BENCHMARKS)
    find_package(benchmark QUIET)
    if(NOT benchmark_FOUND)
        message(STATUS "Google Benchmark not found, downloading...")
        include(FetchContent)

        FetchContent_Declare(
            benchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.3
        )
        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(benchmark)

        # Aplicar flags após fetch para desabilitar warnings rigorosos no Google Benchmark
        if(TARGET benchmark)
            message(STATUS "Configurando flags relaxadas para Google Benchmark")
            target_compile_options(benchmark PRIVATE
                -Wno-conversion
                -Wno-sign-conversion
                -Wno-error
            )
        endif()

        if(TARGET benchmark_main)
            target_compile_options(benchmark_main PRIVATE
                -Wno-conversion
                -Wno-sign-conversion
                -Wno-error
            )
        endif()
    endif()
endif()

if(DOTENV_ENABLE_SANITIZERS)
    add_compile_options(-fno-omit-frame-pointer -fsanitize=address -fsanitize=alignment -fsanitize=bool -fsanitize=bounds -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=pointer-compare -fsanitize=pointer-overflow -fsanitize=pointer-subtract -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=undefined -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr -g)
    add_link_options(-fno-omit-frame-pointer -fsanitize=address -fsanitize=alignment -fsanitize=bool -fsanitize=bounds -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=pointer-compare -fsanitize=pointer-overflow -fsanitize=pointer-subtract -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=undefined -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr -g)

    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message(STATUS "Setting Clang sanitize flags")
        add_compile_options(-fsanitize=implicit-conversion -fsanitize=unsigned-integer-overflow)
        add_link_options(-fsanitize=implicit-conversion -fsanitize=unsigned-integer-overflow)
    endif()
endif()

# Function to filter out excluded files
function(filter_out excluded output)
    set(result "")
    foreach(file ${${output}})
        if(NOT "${file}" IN_LIST ${excluded})
            list(APPEND result ${file})
        endif()
    endforeach()
    set(${output} "${result}" PARENT_SCOPE)
endfunction()

# Add source files - usar lista explícita em vez de GLOB_RECURSE
set(DOTENV_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dotenv.cpp"
)

message(STATUS "DOTENV_SOURCES: ${DOTENV_SOURCES}")

add_library(dotenv_lib STATIC ${DOTENV_SOURCES})

# Usar target_include_directories em vez de include_directories global
target_include_directories(dotenv_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Usar target_compile_features para garantir C++20
target_compile_features(dotenv_lib PUBLIC cxx_std_20)

if (DOTENV_COMPILER_FLAGS)
    target_compile_options(dotenv_lib PRIVATE ${DOTENV_COMPILER_FLAGS})
endif()
if (DOTENV_LINKER_FLAGS)
    target_link_options(dotenv_lib PRIVATE ${DOTENV_LINKER_FLAGS})
endif()

if (DOTENV_BUILD_EXE)
    add_executable(dotenvexe "src/main.cpp")
    target_link_libraries(dotenvexe dotenv_lib)
    target_compile_features(dotenvexe PRIVATE cxx_std_20)

    if (DOTENV_COMPILER_FLAGS)
        target_compile_options(dotenvexe PRIVATE ${DOTENV_COMPILER_FLAGS})
    endif()
    if (DOTENV_LINKER_FLAGS)
        target_link_options(dotenvexe PRIVATE ${DOTENV_LINKER_FLAGS})
    endif()
endif()

# Install targets - modern CMake package config
install(TARGETS dotenv_lib
    EXPORT dotenvTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers preserving directory structure
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Include CMakePackageConfigHelpers for modern package config
include(CMakePackageConfigHelpers)

# Generate package config files
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dotenvConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/dotenvConfig.cmake"
    INSTALL_DESTINATION lib/cmake/dotenv
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/dotenvConfigVersion.cmake"
    VERSION ${DOTENV_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install package config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/dotenvConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/dotenvConfigVersion.cmake"
    DESTINATION lib/cmake/dotenv
)

# Export targets to build tree (for development)
export(EXPORT dotenvTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/dotenvTargets.cmake"
    NAMESPACE dotenv::
)

# Install targets to install tree
install(EXPORT dotenvTargets
    FILE dotenvTargets.cmake
    NAMESPACE dotenv::
    DESTINATION lib/cmake/dotenv
)

# Register package in CMake registry (optional, for development)
if(NOT IS_CONAN)
    export(PACKAGE dotenv)
endif()

if(DOTENV_ENABLE_TESTS)
    add_subdirectory(tests)
endif()

if(DOTENV_ENABLE_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
