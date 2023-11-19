
#include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(cpp-example cpp-example.cpp acutest.h)
add_executable(morton-test test_morton.cpp ../src/Octree.cpp acutest.h)
