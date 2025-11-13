# options
message(STATUS "Building jsonui17 examples: ${JSONUI_APPLICATION}")

#include_directories(${CMAKE_SOURCE_DIR})
find_package(Threads REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Recursively collect all header files from the current directory
file(GLOB_RECURSE APP_HEADER_FILES *.h)

# Recursively collect all source files from the current directory
file(GLOB_RECURSE APP_SOURCE_FILES *.cpp)

add_executable(jsonui17-test ${APP_HEADER_FILES} ${APP_SOURCE_FILES})
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
     target_link_libraries(jsonui17-test jsonui17::jsonui17-static Threads::Threads)
else()
     target_link_libraries(jsonui17-test jsonui17::jsonui17 Threads::Threads)
endif()

