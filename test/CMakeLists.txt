cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

add_executable(thread_pool_test thread_pool_test.c)

target_link_libraries(thread_pool_test thread_pool)
target_include_directories(thread_pool_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/include)

include(CTest)
