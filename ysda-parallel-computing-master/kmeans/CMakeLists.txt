cmake_minimum_required(VERSION 2.8)
project(kmeans)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(kmeans kmeans/kmeans.h kmeans/kmeans.cpp kmeans/main.cpp)
add_executable(data_gen data-gen/data-gen.h data-gen/data-gen.cpp data-gen/main.cpp)
add_executable(test_omp test.cpp kmeans/kmeans.h kmeans/kmeans.cpp data-gen/data-gen.h data-gen/data-gen.cpp)
