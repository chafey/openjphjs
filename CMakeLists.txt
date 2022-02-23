cmake_minimum_required(VERSION 3.16)

project (openjphjs
  LANGUAGES CXX)

# set the build type if not specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}")
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/OpenJPH/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

if(EMSCRIPTEN)
  option(OJPH_DISABLE_INTEL_SIMD "Disables the use of SIMD instructions and associated files" ON)
endif()

option(BUILD_SHARED_LIBS "" OFF)

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fexceptions")

# add the external library
add_subdirectory(extern/OpenJPH EXCLUDE_FROM_ALL)

# add the js wrapper
if(EMSCRIPTEN)
  add_subdirectory(src)
endif()

# c++ native test case
if(NOT EMSCRIPTEN)
  add_subdirectory(test/cpp)
endif()