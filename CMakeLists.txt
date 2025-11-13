# Require a certain version of cmake
cmake_minimum_required(VERSION 3.16)

# Set the name of the project
project(jsonqml VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_MACOSX_RPATH ON)

# Define variables with the GNU standard installation directories
include(GNUInstallDirs)

# Set the cmake module path of the project
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Use ccache to speed up repeated compilations
include(CCache)

# Ensure proper configuration if in a conda environment
include(CondaAware)

# Define which types of libraries to build
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(BuildTools "Build BuildTools" ON)
option(BuildTests "Build test suite" ON)

# Used into conda only
if(DEFINED ENV{CONDA_PREFIX})
   option(USE_SPDLOG_PRECOMPILED "Use spdlog in compiled version" ON)
else()
   option(USE_SPDLOG_PRECOMPILED "Use spdlog in compiled version" OFF)
endif()

# Set the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Currently is not setup to produce a dynamic library using MSVC, only static
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(BUILD_SHARED_LIBS OFF)
endif()

# Set libraries to be compiled with position independent code mode (i.e., fPIC option in GNU compilers)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find Qt dependencies
find_package(Qt6 6.2 COMPONENTS Gui Quick Charts Concurrent Svg PrintSupport Test REQUIRED)
qt_standard_project_setup(REQUIRES 6.8)

# Find other dependencies
include(jsonqmlFindDeps)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
   include_directories("/usr/local/include")
   link_directories("/usr/local/lib")
endif()
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
   include_directories("c:/usr/local/include")
   link_directories("c:/usr/local/lib")
endif()

# Set the jsonqml header directory path
set(JSONQML_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(JSONQML_HEADER_DIR ${CMAKE_SOURCE_DIR}/include)

# Set the include directories
include_directories(${JSONQML_HEADER_DIR})

# Build jsonqml library
add_subdirectory(src)

# Build the examples
if(BuildTools)
    add_subdirectory(tools)
endif()

# Run the tests
if(BuildTests)
    add_subdirectory(tests)
endif()

# Install the cmake config files that permit users to use find_package(jsonqml)
include(jsonqmlInstallCMakeConfigFiles)

