cmake_minimum_required(VERSION 3.12)
project(cmake_integration_example CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Exemplo de como usar dotenv em um projeto CMake
# Método 1: find_package (preferido quando dotenv está instalado)
find_package(dotenv REQUIRED)

# Verificar que o target foi encontrado
if(TARGET dotenv::dotenv_lib)
    message(STATUS "✓ dotenv::dotenv_lib target found")
else()
    message(FATAL_ERROR "✗ dotenv::dotenv_lib target not found")
endif()

# Criar executável
add_executable(cmake_integration_example main.cpp)

# Linkar com dotenv
target_link_libraries(cmake_integration_example
    PRIVATE
        dotenv::dotenv_lib
)

# Demonstrar propriedades do target
get_target_property(DOTENV_INCLUDE_DIRS dotenv::dotenv_lib INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(DOTENV_COMPILE_FEATURES dotenv::dotenv_lib INTERFACE_COMPILE_FEATURES)

message(STATUS "dotenv include directories: ${DOTENV_INCLUDE_DIRS}")
message(STATUS "dotenv compile features: ${DOTENV_COMPILE_FEATURES}")

# Copiar arquivo .env para teste
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.env
    ${CMAKE_CURRENT_BINARY_DIR}/config.env
    COPYONLY
)
