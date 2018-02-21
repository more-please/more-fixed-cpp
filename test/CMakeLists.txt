set(CMAKE_CXX_STANDARD 11)

include_directories(../include)

add_executable(test test.cpp)
add_executable(benchmark benchmark.cpp)
add_executable(test_math test_math.cpp)
