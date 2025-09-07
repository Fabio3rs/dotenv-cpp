cmake_minimum_required(VERSION 3.12)
project(cmake_integration_example CXX)

# Use the same C++ standard as the main project
if(DEFINED DOTENV_DETECTED_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD ${DOTENV_DETECTED_CXX_STANDARD})
    message(STATUS "🔧 Using detected C++ standard: ${DOTENV_DETECTED_CXX_STANDARD}")
else()
    set(CMAKE_CXX_STANDARD 20)
    message(STATUS "🔧 Using fallback C++ standard: 20")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Exemplo de como usar dotenv em um projeto CMake
# Método 1: find_package (preferido quando dotenv está instalado)
# But only try if we're building standalone
if(NOT TARGET dotenv_lib)
    find_package(dotenv REQUIRED)
endif()

# Verificar que o target foi encontrado
if(TARGET dotenv::dotenv_lib)
    message(STATUS "✓ dotenv::dotenv_lib target found (installed)")
    set(DOTENV_TARGET dotenv::dotenv_lib)
elseif(TARGET dotenv_lib)
    message(STATUS "✓ dotenv_lib target found (local)")
    set(DOTENV_TARGET dotenv_lib)
else()
    message(FATAL_ERROR "✗ No dotenv target found")
endif()

# Criar executável
add_executable(cmake_integration_example main.cpp)

# Linkar com dotenv
target_link_libraries(cmake_integration_example
    PRIVATE
        ${DOTENV_TARGET}
)

# Demonstrar propriedades do target
get_target_property(DOTENV_INCLUDE_DIRS ${DOTENV_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(DOTENV_COMPILE_FEATURES ${DOTENV_TARGET} INTERFACE_COMPILE_FEATURES)

message(STATUS "dotenv include directories: ${DOTENV_INCLUDE_DIRS}")
message(STATUS "dotenv compile features: ${DOTENV_COMPILE_FEATURES}")

# Copiar arquivo .env para teste
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.env
    ${CMAKE_CURRENT_BINARY_DIR}/config.env
    COPYONLY
)
