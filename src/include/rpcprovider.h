#pragma once
#include "google/protobuf/service.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "mprpcapplication.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpConnection.h"
#include "rpcheader.pb.h"
#include <string>
#include <iostream>
#include <unordered_map>
// 该类负责发布rpc方法



class RpcProvider 
{
public:
    // 发布rpc方法的接口，供框架使用者调用
    void NotifyService(google::protobuf::Service* service);

    // 启动rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    void Run();
private:
    
    void OnConnection(const muduo::net::TcpConnectionPtr& conn); // 连接回调函数
    // 已建立连接用户的读写回调函数
    void OnMessage(const muduo::net::TcpConnectionPtr& ,muduo::net::Buffer*,muduo::Timestamp);
    // Closure是google::protobuf中的一个类，表示一个回调函数对象，Run()方法会执行回调函数对象中存储的函数
    void sendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response);

    enum class ParseResult
    {
        kComplete = 0,
        kInComplete = 1,
        kError = 2,
    };

    ParseResult parseRpcMessage(muduo::net::Buffer* buf, mprpc::RpcHeader* rpc_header, std::string& args_str);
    muduo::net::EventLoop m_eventLoop; // muduo网络库，事件循环
    // 存储服务信息
    struct ServiceInfo
    {
        google::protobuf::Service* service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap; // 存储方法名称和方法描述信息的键值对
    };
    std::unordered_map<std::string, ServiceInfo> m_serviceInfoMap; // 存储服务名称和服务信息的键值对
};