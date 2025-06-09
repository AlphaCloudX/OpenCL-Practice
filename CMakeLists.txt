cmake_minimum_required(VERSION 3.29)
project(OpenClTextBook C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Header path (your local include folder)
include_directories("${CMAKE_SOURCE_DIR}/include")

# Explicit path to the OpenCL library file
set(OPENCL_LIBRARY "${CMAKE_SOURCE_DIR}/OpenCL.lib")

# Add the main executable
add_executable(OpenClTextBook LearnToQueryPlatform.c)

# Link with OpenCL manually
target_link_libraries(OpenClTextBook PRIVATE ${OPENCL_LIBRARY})

# Messages for confirmation
message(STATUS "Using OpenCL library: ${OPENCL_LIBRARY}")
message(STATUS "Include path: ${CMAKE_SOURCE_DIR}/include")
