set(SRC_LIST
    UserService.cc
    ../user.pb.cc
)
set(CMAKE_BUILD_TYPE "Debug")
add_executable(provider ${SRC_LIST})

target_link_libraries(provider mprpc protobuf)