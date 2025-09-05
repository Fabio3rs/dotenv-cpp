cmake_minimum_required(VERSION 3.12)
project(basic_usage_example CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tentar encontrar dotenv via find_package primeiro
find_package(dotenv QUIET)

if(dotenv_FOUND)
    message(STATUS "Using installed dotenv library")
    add_executable(basic_usage_example main.cpp)
    target_link_libraries(basic_usage_example dotenv::dotenv_lib)
else()
    message(STATUS "dotenv not found, using local build")
    # Fallback: adicionar o diretório pai como subproject
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. dotenv_build)
    add_executable(basic_usage_example main.cpp)
    target_link_libraries(basic_usage_example dotenv_lib)
    target_include_directories(basic_usage_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
endif()

# Copiar example.env para o build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/example.env
    ${CMAKE_CURRENT_BINARY_DIR}/example.env
    COPYONLY
)
