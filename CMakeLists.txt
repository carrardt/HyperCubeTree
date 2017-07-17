cmake_minimum_required (VERSION 3.0)
project (HyperCubeTree)

add_compile_options(-std=c++14)
add_definitions(-DPROFILING=1)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(reader)
add_subdirectory(tests)
add_subdirectory(amr2ugrid)
add_subdirectory(hydro)
