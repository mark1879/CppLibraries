cmake_minimum_required(VERSION 3.1)

project(thread_pool)

add_library(thread_pool thread_pool.cpp)

# MESSAGE("<<<<<< thread_pool")
# MESSAGE(${CMAKE_SOURCE_DIR})
# MESSAGE(${CMAKE_INSTALL_PREFIX})
# MESSAGE(${PROJECT_SOURCE_DIR})
# MESSAGE(${PROJECT_BINARY_DIR})

set(head_files thread_pool.hpp ${CMAKE_SOURCE_DIR}/common/logger.hpp)
install(FILES ${head_files} DESTINATION  ${PROJECT_NAME}/include)
install(TARGETS thread_pool ARCHIVE DESTINATION ${PROJECT_NAME}/lib)

if(${BUILD_DEMO} STREQUAL "true")
    add_subdirectory(examples/)
endif()
