cmake_minimum_required(VERSION 3.10)
project(mprpc)
set(CMAKE_CXX_STANDARD 20)
# 设置编译输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


include_directories(${PROJECT_SOURCE_DIR}/example) # -I
include_directories(${PROJECT_SOURCE_DIR}/src/include) # -I

link_directories(${PROJECT_SOURCE_DIR}/lib)    # -L

add_subdirectory(src)
add_subdirectory(example)