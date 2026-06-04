set(SRC_LIST
    calluserservice.cc
    ../user.pb.cc
)
set(CMAKE_BUILD_TYPE "Debug")
add_executable(consumer ${SRC_LIST})

target_link_libraries(consumer mprpc protobuf)