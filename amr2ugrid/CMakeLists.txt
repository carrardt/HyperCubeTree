include_directories(${CMAKE_SOURCE_DIR}/reader)

add_executable(MeshInfo MeshInfo.cc)
target_link_libraries(MeshInfo MeshReader)

add_executable(AmrLevels AmrLevels.cc)
target_link_libraries(AmrLevels MeshReader)

add_executable(AmrTree AmrTree.cc)
target_link_libraries(AmrTree MeshReader)

add_executable(AmrConnect AmrConnect.cc)
target_link_libraries(AmrConnect MeshReader)

add_executable(AmrSidePoints AmrSidePoints.cc)
target_link_libraries(AmrSidePoints MeshReader)
