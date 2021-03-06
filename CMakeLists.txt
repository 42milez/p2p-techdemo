cmake_minimum_required(VERSION 3.10.2)

include(cmake/cable/bootstrap.cmake)
include(CableBuildInfo)
include(CableBuildType)
include(CableToolchains)
cable_configure_toolchain(DEFAULT cxx17)
cable_set_build_type(DEFAULT RelWithDebInfo CONFIGURATION_TYPES Debug Release RelWithDebInfo)

project(StarNetwork)
set(PROJECT_VERSION 0.0.1)
cable_add_buildinfo_library(PROJECT_NAME star_network)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(
        BASIC_SETUP
        BUILD missing
        CONANFILE conanfile.txt
        ENV CC=/usr/bin/clang-9
        ENV CXX=/usr/bin/clang++-9
        SETTINGS arch=x86_64
        SETTINGS compiler=clang
        SETTINGS compiler.libcxx=libstdc++11
        SETTINGS compiler.version=9
        SETTINGS os=Linux)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -DNDEBUG")

option(VERBOSE_LOGGING "Enable verbose logging" OFF)

if(VERBOSE_LOGGING)
  add_definitions(-DVERBOSE_LOGGING)
endif()

#  Directories
# --------------------------------------------------
include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(app)
add_subdirectory(lib)

#  Directories (Tests)
# --------------------------------------------------
add_subdirectory(test)
