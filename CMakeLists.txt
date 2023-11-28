cmake_minimum_required(VERSION 3.20.0)

project(rvemu LANGUAGES C)

set(CMAKE_C_STANDARD 17)

set(C_FILES
  ${PROJECT_SOURCE_DIR}/src/decode.c
  ${PROJECT_SOURCE_DIR}/src/interp.c
  ${PROJECT_SOURCE_DIR}/src/machine.c
  ${PROJECT_SOURCE_DIR}/src/main.c
  ${PROJECT_SOURCE_DIR}/src/mmu.c
  ${PROJECT_SOURCE_DIR}/src/syscall.c
)

add_executable(${PROJECT_NAME} ${C_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
  src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  m
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
)

target_compile_options(${PROJECT_NAME} PRIVATE
  -O3
  -Wall
  -Werror
  -Wimplicit-fallthrough
)
