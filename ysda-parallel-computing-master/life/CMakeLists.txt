cmake_minimum_required(VERSION 2.8)
project(life)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

find_package(MPI REQUIRED)

include_directories(${MPI_INCLUDE_PATH})

add_executable(life main.c)
add_executable(data-gen data-gen.c)
add_executable(consequent consequent.c)

target_link_libraries(life ${MPI_LIBRARIES})

if(MPI_COMPILE_FLAGS)
    set_target_properties(life PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
    set_target_properties(life PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()
