cmake_minimum_required(VERSION 3.10)

project(ThreadPoolLibrary C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# set(CMAKE_C_COMPILER gcc)

# set(ASAN_FLAGS "-fsanitize=address,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr")

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fstack-protector-strong -fstrict-overflow")
# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og -g3 -DDEBUG -ggdb -D_FORTIFY_SOURCE=3 ${ASAN_FLAGS}")
# set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -march=native -flto -g3 -DNDEBUG")

# set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Og -g3 -DDEBUG -ggdb -D_FORTIFY_SOURCE=3 ${ASAN_FLAGS}")
# set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -O2 -march=native -flto -g3 -DNDEBUG")

# Create the library
add_library(thread_pool STATIC # or SHARED for a shared library
    src/source/thread_pool.c
)

# Set include directories for the library
target_include_directories(thread_pool PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include> # For building
    $<INSTALL_INTERFACE:include> # For installation
)

# Installation rules (optional, but recommended)
install(TARGETS thread_pool
    EXPORT thread_pool-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY src/include/
    DESTINATION include
)

# Generate export file for find_package()
install(EXPORT thread_pool-targets
    FILE thread_pool-config.cmake
    NAMESPACE thread_pool::
    DESTINATION lib/cmake/thread_pool
)

# Create a config file for find_package
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/thread_pool-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/thread_pool-config.cmake
    COPYONLY
)

# Example thread_pool-config.cmake.in file
# This file will be configured by CMake
# It is used by find_package() to locate the library
# and its dependencies

# Set the install prefix, which is used to
# relocate the install directory of the
# thread pool library
set(thread_pool_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the library name
set(thread_pool_LIB_NAME "thread_pool")

# Set the include directories
set(thread_pool_INCLUDE_DIRS "${thread_pool_INSTALL_PREFIX}/include")

# Set the library locations
set(thread_pool_LIBRARY_DIRS "${thread_pool_INSTALL_PREFIX}/lib")

# Set the library targets
set(thread_pool_LIBRARIES "thread_pool")

if(BUILD_TESTS)
    # test subdir
    add_subdirectory(test)
endif()
