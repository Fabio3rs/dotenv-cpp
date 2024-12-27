cmake_minimum_required(VERSION 3.1)
project(PackageTest CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

message(STATUS "CONAN_LIBS: ${CONAN_LIBS}")
message(STATUS "CONAN_LIBS_RELEASE: ${CONAN_LIBS_RELEASE}")

# debug include dirs
message(STATUS "CONAN_INCLUDE_DIRS: ${CONAN_INCLUDE_DIRS}")

add_executable(example example.cpp)
target_link_libraries(example ${CONAN_LIBS})

# copy .env after build
add_custom_command(TARGET example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/.env
    $<TARGET_FILE_DIR:example>/.env
)

# CTest is a testing tool that can be used to test your project.
# enable_testing()
# add_test(NAME example
#          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#          COMMAND example)
