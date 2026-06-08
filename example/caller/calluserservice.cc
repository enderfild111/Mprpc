#include <iostream>
#include "user.pb.h"
#include "mprpcapplication.h"
#include <chrono>
#include "mprpcLog.h"

int main(int argc, char* argv[])
{
    MprpcApplication::Init(argc,argv); // 调用框架初始化操作

    // 调用rpc远程发布的方法login
    example::UserServiceRpc_Stub stub(new MprpcChannel()); // rpcchannel对象需要框架提供，负责数据的序列化和网络传输
    example::LoginRequest request;
    // 模拟打包
    request.set_name("hpz");
    request.set_pwd("123456");
    example::LoginResponse response;
    MprpcController controller;
    std::cout << "call begin: " << std::chrono::system_clock::now() << std::endl;
    stub.Login(&controller, &request, &response, nullptr);
    std::cout << "call end: " << std::chrono::system_clock::now() << std::endl;
    auto xx = std::filesystem::current_path();
    std::cout << "current path: " << xx << std::endl;
    if(controller.Failed())
    {
        LOG(ERROR) << controller.ErrorText();
        exit(EXIT_FAILURE);
    }
    if(0 == response.result().errcode())
    {
        LOG(INFO) << "Login Response:" << response.success();
    }
    else
    {
        LOG(ERROR) << "Login Failed! errcode: " 
                   << std::to_string(response.result().errcode()) 
                   << " errmsg: " << response.result().errmsg();
    }
    return 0;
}