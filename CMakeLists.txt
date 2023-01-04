cmake_minimum_required(VERSION 3.24)

# ------------- integrate vcpkg -------------------
# NOTE: This must be defined before the first project call
# The CMAKE_TOOLCHAIN_FILE variable is not used until the project() command is reached
if(DEFINED CACHE{CMAKE_TOOLCHAIN_FILE})
    message("---------CMAKE_TOOLCHAIN_FILE is cached")
else()
    message("---------integrating vcpkg")
    set(VCPKG_CMAKE_TOOLCHAIN_PILE ${CMAKE_CURRENT_SOURCE_DIR}/build/fetched/vcpkg/scripts/buildsystems/vcpkg.cmake)

    if(NOT EXISTS ${VCPKG_CMAKE_TOOLCHAIN_PILE})
        include(FetchContent)
        FetchContent_Declare(vcpkg
            GIT_REPOSITORY https://github.com/microsoft/vcpkg/
            GIT_TAG 2022.11.14
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/fetched/vcpkg
        )
        FetchContent_populate(vcpkg)
    endif()

    set(CMAKE_TOOLCHAIN_FILE ${VCPKG_CMAKE_TOOLCHAIN_PILE} CACHE FILEPATH "")
    set(VCPKG_INSTALLED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/fetched/vcpkg_installed CACHE FILEPATH "")
endif()

# ------------- CppGen -------------------
project(CppGen)

file(GLOB_RECURSE HEADERS "source/cppgen/*.h" "include/*.h")
file(GLOB_RECURSE SOURCES "source/cppgen/*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADERS} ${SOURCES})
add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)

# The package llvm provides CMake targets:
option(HACK_USE_LIBCLANG_BINARY "Do not set, it's just a hack" OFF)
if(HACK_USE_LIBCLANG_BINARY)
include(build/fetched/libclang-bin/libclang.cmake)
else()
find_package(Clang REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE libclang)
endif()
# magic-enum provides CMake targets:

# this is heuristically generated, and may not be correct
find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE magic_enum::magic_enum)

# yaml-cpp provides CMake targets:

# this is heuristically generated, and may not be correct
find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)

# ------------- executable -------------
add_executable(${PROJECT_NAME}c source/main.cpp)
target_link_libraries(${PROJECT_NAME}c PRIVATE ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}c PROPERTIES CXX_STANDARD 20)

# ------------- test_codeGenerator -------------
add_executable(test_codeGenerator test/test_codeGenerator.cpp)
target_include_directories(test_codeGenerator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(test_codeGenerator PROPERTIES CXX_STANDARD 20)

# ------------- test_cppgen -------------
add_executable(test_cppgen test/test_cppgen.cpp)
target_link_libraries(test_cppgen PRIVATE ${PROJECT_NAME})
set_target_properties(test_cppgen PROPERTIES CXX_STANDARD 20)
target_compile_definitions(test_cppgen PRIVATE CPPGEN_TEST_DIR="${CMAKE_CURRENT_BINARY_DIR}")