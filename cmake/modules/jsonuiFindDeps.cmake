find_package(arango-cpp REQUIRED)
if(NOT arango-cpp_FOUND)
    message(FATAL_ERROR "arango-cpp library not found")
endif()

if(USE_SPDLOG_PRECOMPILED)
   if(NOT TARGET spdlog::spdlog)
       find_package(spdlog CONFIG REQUIRED)
       if(NOT spdlog_FOUND)
           message(FATAL_ERROR "spdlog not found")
       endif()
   endif()
endif()

find_package(jsonio REQUIRED)
if(NOT jsonio_FOUND)
    message(FATAL_ERROR "jsonio library not found")
endif()

