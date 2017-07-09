cmake_minimum_required (VERSION 3.0)
project (HyperCubeTree)

add_compile_options(-std=c++11 -pthread)
add_definitions(-DPROFILING=1)

add_subdirectory(tests)
