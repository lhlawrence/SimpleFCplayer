cmake_minimum_required(VERSION 3.16)
project(simpleFCPlayer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译器标志
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread -fopenmp -s")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pthread -fopenmp -s")

find_package(glfw3 REQUIRED)
# 包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}
    /usr/local/include # 指定 GLAD 的 include 目录
    /usr/include/GLFW # 指定 GLFW 的 include 目录
)

# 链接目录


# 添加可执行文件
add_executable(simpleFCPlayer 
    main.cpp
    src/glad.c # 包含 GLAD 源文件
)

# 链接库
target_link_libraries(simpleFCPlayer 
    glfw
    dl # 通常需要链接 dl 库，特别是在 Linux 系统上
    pthread
)
