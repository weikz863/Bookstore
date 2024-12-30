cmake_minimum_required(VERSION 3.22)
project(Bookstore)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
  ${PROJECT_SOURCE_DIR}/include/
)

file(GLOB_RECURSE main_src src/*.cpp)

add_executable(code ${main_src})