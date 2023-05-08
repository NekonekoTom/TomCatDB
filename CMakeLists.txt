cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME "TomCatDB")

project(${PROJECT_NAME})
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 11)
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
  # set (CMAKE_CXX_FLAGS "-std=c++11 -Wall -Werror -fPIC -g -O0 ${CMAKE_CXX_FLAGS}")
  set (CMAKE_CXX_FLAGS "-std=c++11 -fPIC -g -O0 ${CMAKE_CXX_FLAGS}")
else ()
  set (CMAKE_CXX_FLAGS "-std=c++11 -Wall -fPIC -O2 ${CMAKE_CXX_FLAGS}")
endif ()

# Generate compile_commands.json for clangd, for C++ on VSCode
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_OUTPUT_PATH ${ROOT_DIR}/bin)

include_directories(
  ${ROOT_DIR}/include
  ${ROOT_DIR}/include/coding
  ${ROOT_DIR}/include/io
  ${ROOT_DIR}/include/table
  ${ROOT_DIR}/include/util
)

link_directories(${ROOT_DIR}/lib)

add_subdirectory("src")