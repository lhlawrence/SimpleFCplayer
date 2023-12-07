cmake_minimum_required(VERSION 3.16)
include(cross.cmake)
project(simpleFCPlayer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -pthread -fopenmp -s")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pthread -fopenmp -s")

include_directories(
    ${PROJECT_SOURCE_DIR}
)

link_directories(
    ${CMAKE_FIND_ROOT_PATH}/lib
)



add_executable(simpleFCPlayer 
    main.cpp
)

target_link_libraries(simpleFCPlayer 
    GL
    glut
)