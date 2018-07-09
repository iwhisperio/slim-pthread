cmake_minimum_required(VERSION 2.9)
cmake_policy(VERSION 2.9)
project(slim-pthread-tests C)

include_directories(../src include)
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_POSIX_THREAD_PRIORITY_SCHEDULING)

if(${ENABLE_STATIC})
  set(PTHREAD pthread-static)
else()
  set(PTHREAD pthread-shared)
endif()

file(GLOB_RECURSE TESTS *.c)

foreach(SRC ${TESTS})
  get_filename_component(CASE_NUMBER ${SRC} NAME_WE)
  get_filename_component(_DIR_NAME ${SRC} DIRECTORY)
  get_filename_component(CASE_API ${_DIR_NAME} NAME)
  get_filename_component(_DIR_NAME ${_DIR_NAME} DIRECTORY)
  get_filename_component(CASE_CAT ${_DIR_NAME} NAME)
  get_filename_component(_DIR_NAME ${_DIR_NAME} DIRECTORY)
  get_filename_component(CASE_TYPE ${_DIR_NAME} NAME)

  string(SUBSTRING ${CASE_CAT} 0 1 CASE_CAT)
  string(SUBSTRING ${CASE_TYPE} 0 1 CASE_TYPE)

  set(TEST_NAME "${CASE_TYPE}-${CASE_CAT}-${CASE_API}-${CASE_NUMBER}")

  add_executable(${TEST_NAME} ${SRC})
  target_link_libraries(${TEST_NAME} ${PTHREAD})
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach(SRC)
