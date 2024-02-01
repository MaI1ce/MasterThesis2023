cmake_minimum_required(VERSION 3.10)

add_compile_options(-O3 -Wall -Wextra)

project(ds2)

option (IS_IOS "Build for iOS." OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)