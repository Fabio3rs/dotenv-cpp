cmake_minimum_required(VERSION 3.12)
project(dotenv C CXX)

set(IS_CONAN FALSE)
get_cmake_property(all_variables CACHE_VARIABLES)
foreach (var ${all_variables})
    if ("${var}" MATCHES "CONAN")
        set(IS_CONAN TRUE)
    endif()
endforeach()

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}" AND NOT IS_CONAN)
    message(STATUS "Setting up dotenv project")
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_C_EXTENSIONS OFF)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        message(STATUS "Setting G++ flags")
        add_compile_options(-Wall -Werror -Wextra -Wformat-security -Wconversion -Wsign-conversion  -Wno-gnu -Wno-gnu-statement-expression)
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        message(STATUS "Setting MSVC flags")
        add_compile_options(/W4 /WX)
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message(STATUS "Setting Clang flags")
        add_compile_options(-Werror -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-global-constructors -Wno-exit-time-destructors -Wno-weak-vtables -Wno-documentation -Wno-documentation-unknown-command)
    endif()

    option(DOTENV_ENABLE_TESTS "Enable tests" ON)
    option(DOTENV_ENABLE_SANITIZERS "Enable sanitizers" ON)
    option(DOTENV_BUILD_EXE "Enable a simple dotenv test executable" ON)
else()
    option(DOTENV_ENABLE_TESTS "Enable tests" OFF)
    option(DOTENV_ENABLE_SANITIZERS "Enable sanitizers" OFF)
    option(DOTENV_BUILD_EXE "Enable a simple dotenv test executable" OFF)
endif()

if (IS_CONAN)
set(CMAKE_CXX_STANDARD 17)
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

if(DOTENV_ENABLE_SANITIZERS)
    add_compile_options(-fno-omit-frame-pointer -fsanitize=address -fsanitize=alignment -fsanitize=bool -fsanitize=bounds -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=pointer-compare -fsanitize=pointer-overflow -fsanitize=pointer-subtract -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=undefined -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr -g)
    add_link_options(-fno-omit-frame-pointer -fsanitize=address -fsanitize=alignment -fsanitize=bool -fsanitize=bounds -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=pointer-compare -fsanitize=pointer-overflow -fsanitize=pointer-subtract -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=undefined -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr -g)

    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message(STATUS "Setting Clang sanitize flags")
        add_compile_options(-fsanitize=implicit-conversion -fsanitize=unsigned-integer-overflow)
        add_link_options(-fsanitize=implicit-conversion -fsanitize=unsigned-integer-overflow)
    endif()
endif()

# Add include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

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

# Add source files
file(GLOB_RECURSE DOTENV_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Exclude specific files
set(EXCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
filter_out(EXCLUDE_FILES DOTENV_SOURCES)

message(STATUS "DOTENV_SOURCES: ${DOTENV_SOURCES}")

add_library(dotenv_lib STATIC ${DOTENV_SOURCES})

if (DOTENV_COMPILER_FLAGS)
    target_compile_options(dotenv_lib PRIVATE ${DOTENV_COMPILER_FLAGS})
endif()
if (DOTENV_LINKER_FLAGS)
    target_link_options(dotenv_lib PRIVATE ${DOTENV_LINKER_FLAGS})
endif()

if (DOTENV_BUILD_EXE)
    add_executable(dotenvexe "src/main.cpp")
    target_link_libraries(dotenvexe dotenv_lib)

    if (DOTENV_COMPILER_FLAGS)
        target_compile_options(dotenvexe PRIVATE ${DOTENV_COMPILER_FLAGS})
    endif()
    if (DOTENV_LINKER_FLAGS)
        target_link_options(dotenvexe PRIVATE ${DOTENV_LINKER_FLAGS})
    endif()
endif()

# Install targets
install(TARGETS dotenv_lib
    EXPORT dotenvTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/*
    DESTINATION include
)

# Export package config
install(EXPORT dotenvTargets
    FILE dotenvConfig.cmake
    NAMESPACE dotenv::
    DESTINATION lib/cmake/dotenv
)

if(DOTENV_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
