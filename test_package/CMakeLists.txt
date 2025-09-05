cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the dotenv package
find_package(dotenv REQUIRED)

add_executable(example example.cpp)
target_link_libraries(example dotenv::dotenv)

# Copy .env file to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/.env
    ${CMAKE_CURRENT_BINARY_DIR}/.env
    COPYONLY
)

# CTest is a testing tool that can be used to test your project.
# enable_testing()
# add_test(NAME example
#          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#          COMMAND example)
