cmake_minimum_required(VERSION 3.8)

project(locktrack)
set(LOCKTRACK_VERSION "0.5")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
  "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
  FORCE)
  message(STATUS "Build type not specified. Setting to: ${CMAKE_BUILD_TYPE}")
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_FLAGS_DEBUG "-Werror -g3 -O2 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-g -O3")

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(Doxyfile.in Doxyfile @ONLY)
  add_custom_target(doc
    ALL
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()

include_directories(liblocktrack)

add_subdirectory(benchmarks)
add_subdirectory(liblocktrack)
