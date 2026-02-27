# Require a certain version of cmake
cmake_minimum_required(VERSION 3.16..4.2)

# Set the name of the project
project(jsonui VERSION 1.0.0 LANGUAGES CXX)

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
option(BUILD_APP "Build application." ON)
option(DEPLOY_APP "Deploy application." ON)

# Used into conda only
if(DEFINED ENV{CONDA_PREFIX})
   option(USE_SPDLOG_PRECOMPILED "Use spdlog in compiled version" ON)
else()
   option(USE_SPDLOG_PRECOMPILED "Use spdlog in compiled version" OFF)
endif()

# Set the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    # The build type selection for the project
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type for ${PROJECT_NAME}." FORCE)
    # The build type options for the project
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)
endif()

# Currently is not setup to produce a dynamic library using MSVC, only static
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(BUILD_SHARED_LIBS OFF)
else()
#    set(BUILD_STATIC_LIBS OFF)
endif()

if(BUILD_APP)
   set(BUILD_STATIC_LIBS ON)
endif()

# Set libraries to be compiled with position independent code mode (i.e., fPIC option in GNU compilers)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find other dependencies
include(jsonuiFindDeps)

# Find Qt dependencies
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts Concurrent PrintSupport Svg Sql)
qt_standard_project_setup()

# Versioning
set(JSONUI_VERSION "${PROJECT_VERSION}")
find_package(Git)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE JSONUI_VERSION_HASH
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE JSONUI_GIT_BRANCH
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT JSONUI_VERSION_HASH)
        set(JSONUI_VERSION_HASH "---")
    endif()
    if(NOT JSONUI_GIT_BRANCH)
        set(JSONUI_GIT_BRANCH "---")
    endif()
else()
  set(JSONUI_VERSION_HASH "<unknown>")
  set(JSONUI_GIT_BRANCH "<unknown>")
endif()

set(JSONUI_OSX "${CMAKE_SYSTEM_NAME}")
set(JSONUI_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
set(JSONUI_COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION}")
set(JSONUI_QT_VERSION "${Qt6_VERSION}")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/jsonui_version.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/jsonui_version.h"
  @ONLY
  )

# Set the list of compiler flags for Clang compiler
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    add_compile_options(-Wall -Wno-ignored-attributes -Wno-pedantic -Wno-variadic-macros -Wno-deprecated)
endif()

# Set the list of compiler flags for Intel compiler
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
    add_compile_options(-Wall -Wno-stringop-truncation -Wno-array-bounds -Wno-stringop-overflow)
endif()

# Set the list of compiler flags for MSVC compiler
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_compile_options(
        /D_SCL_SECURE_NO_WARNINGS
        /D_CRT_SECURE_NO_WARNINGS=1
        /MP4
        /EHsc
        /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
        /DNOMINMAX
    )
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
   include_directories("/usr/local/include")
   link_directories("/usr/local/lib")
endif()
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
   include_directories("c:/usr/local/include")
   link_directories("c:/usr/local/lib")
endif()

# Set some necessary definitions
add_definitions(-DIMPEX_OFF)

# Set the jsonui header directory path
set(JSONUI_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(JSONUI_HEADER_DIR ${CMAKE_SOURCE_DIR}/include)

# Set the include directories
include_directories(${JSONUI_HEADER_DIR})

# Build jsonui library
add_subdirectory(src)

# Build the application
if(BUILD_APP)
    add_subdirectory(App)
endif()

# Install the cmake config files that permit users to use find_package(jsonui)
include(jsonuiInstallCMakeConfigFiles)

