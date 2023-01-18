# MESSAGE("<<<<<< examples")
# MESSAGE(${CMAKE_SOURCE_DIR})
# MESSAGE(${CMAKE_INSTALL_PREFIX})
# MESSAGE(${PROJECT_SOURCE_DIR})
# MESSAGE(${PROJECT_BINARY_DIR})

include_directories(${PROJECT_SOURCE_DIR})

add_executable(test_thread_pool test_thread_pool.cpp)

target_link_libraries(test_thread_pool ${PROJECT_BINARY_DIR}/libthread_pool.a)
