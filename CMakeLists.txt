cmake_minimum_required(VERSION 3.20.0)

project(rvemu LANGUAGES C)

set(CMAKE_C_STANDARD 17)

add_compile_options(-O3 -Wall -Werror -Wimplicit-fallthrough)

file(GLOB_RECURSE C_FILES
  ${PROJECT_SOURCE_DIR}/src/*.c
)

add_executable(${PROJECT_NAME} ${C_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
  src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  m
)
