project(locktrack)
set(LOCKTRACK_VERSION "0.5")

cmake_minimum_required(VERSION 3.8)

if(DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING
    "The type of build: Debug, Optimized")
else()
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
    "The type of build: Debug, Optimized")
endif()

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY
  )
  add_custom_target(doc
    ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()

set(CMAKE_C_FLAGS "-Wall ${CONFIG_DEFINES}")
set(CMAKE_C_FLAGS_DEBUG "-Werror -ansi -g3 -O2 -DDEBUG")
set(CMAKE_C_FLAGS_OPTIMIZED "-g -O3")

include_directories(liblocktrack)

add_subdirectory(benchmarks)
add_subdirectory(liblocktrack)
