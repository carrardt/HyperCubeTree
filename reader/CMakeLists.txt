add_library(MeshReader readMesh.cc)

add_executable(readMesh-unit-test readMesh-unit-test.cc)
target_link_libraries(readMesh-unit-test MeshReader)
