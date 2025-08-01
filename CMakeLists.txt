# ==============================================
# Project Definition and options
# ==============================================
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(source_gen)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Define the link libraries
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Define the include DIRs
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/cppast/include"
)
