#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
// 假设UserService 提供的是一个进程内的本地服务
class UserService : public example::UserServiceRpc
{
public:
    bool Login(const std::string& username, const std::string& pwd)
    {
        // 简单的登录逻辑，实际应用中会更复杂
        std::cout << "UserService: Logging in user " << username << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        return true;
    }

    bool Register(uint32_t id, const std::string& name, const std::string& pwd)
    {
        std::cout << id <<"UserService: Register in user" << name << std::endl;
        return true;
    }
    // 重写基类Login方法
    void Login(google::protobuf::RpcController* controller,
                       const ::example::LoginRequest* request,
                       ::example::LoginResponse* response,
                       ::google::protobuf::Closure* done) override
    {
        // 从请求中获取用户名和密码
        if(Login(request->name(), request->pwd()))
        {
            response->set_success(true);
            response->mutable_result()->set_errcode(0); // 登录成功
            response->mutable_result()->set_errmsg("Login successful");
            done->Run(); // 执行回调操作，执行响应对象数据的序列化和发送
        }
        else
        {
            response->set_success(false);
            response->mutable_result()->set_errcode(1); // 登录失败
            response->mutable_result()->set_errmsg("Invalid username or password");
            done->Run(); // 执行回调操作，执行响应对象数据的序列化和发送
        }
    }
    void Register(google::protobuf::RpcController* controller,
                       const ::example::RegisterRequest* request,
                       ::example::RegisterResponse* response,
                       ::google::protobuf::Closure* done) override
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        bool ret = Register(id, name, pwd);
        auto res = response->mutable_result();
        res->set_errcode(0);
        res->set_errmsg("null");
        response->set_success(ret);
        done->Run();
    }                 
};

int main(int argc, char* argv[])
{
    // 调用框架初始化操作
    MprpcApplication::Init(argc, argv);
    // 发布服务,Provider 是一个网络服务对象，负责数据的序列化和网络传输
    RpcProvider provider;
    // 把UserService对象发布到rpc节点上，框架会将UserService对象的地址和类型信息自动注册到ZooKeeper上
    provider.NotifyService(new UserService()); // 把UserService对象发布到rpc节点上
    // provider.NotifyService(new Goods()); // 发布多个服务对象

    provider.Run(); // 启动rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    return 0;
}