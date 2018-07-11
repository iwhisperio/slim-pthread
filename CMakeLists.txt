cmake_minimum_required(VERSION 2.9)
cmake_policy(VERSION 2.9)
project(slim-pthread-win C)

if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  message("Slim pthread for Windows can build on Windows only!")
  exit(1)
endif()

set(ENABLE_SHARED TRUE CACHE BOOL "Build shared library")
set(ENABLE_STATIC FALSE CACHE BOOL "Build static library")
set(ENABLE_TESTS TRUE CACHE BOOL "Build test cases")

add_subdirectory(src)

if(${ENABLE_TESTS})
  enable_testing()
  add_subdirectory(tests)
endif()
