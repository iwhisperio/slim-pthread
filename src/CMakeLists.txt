cmake_minimum_required(VERSION 2.9)
cmake_policy(VERSION 2.9)
project(slim-pthread C)

set(SRC
    pthread.c
    pthread_key.c
    pthread_barrier.c
    pthread_cond.c
    pthread_mutex.c
    pthread_once.c
    pthread_rwlock.c
    dllmain.c)

set(HEADERS pthread.h)

add_definitions(-DSLIM_PTHREAD_BUILD)

if(${ENABLE_STATIC})
  add_library(pthread-static STATIC ${SRC})
  target_compile_definitions(pthread-static PRIVATE SLIM_PTHREAD_STATIC)
  set_target_properties(pthread-static PROPERTIES OUTPUT_NAME pthread)

  install(TARGETS pthread-static
          RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
          ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
          LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
endif()

if(${ENABLE_SHARED})
  add_library(pthread-shared SHARED ${SRC})
  target_compile_definitions(pthread-shared PRIVATE SLIM_PTHREAD_DYNAMIC)
  set_target_properties(pthread-shared PROPERTIES OUTPUT_NAME pthread)

  install(TARGETS pthread-shared
          RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
          ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
          LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
endif()

install(FILES ${HEADERS}
        DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
