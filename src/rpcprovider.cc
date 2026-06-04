#include "rpcprovider.h"
#include <functional>
#include<arpa/inet.h> // for htonl and ntohl
// 发布rpc方法的接口，供框架使用者调用
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    auto descriptor = service->GetDescriptor(); // 获取服务对象的描述信息
    ServiceInfo service_info; // 定义一个服务信息对象
    service_info.service = service; // 存储服务对象
    std::string service_name = descriptor->name(); // 获取服务名称
    std::size_t method_count = descriptor->method_count(); // 获取服务方法数量
    for(size_t i = 0; i < method_count; ++i)
    {
        const google::protobuf::MethodDescriptor* method_descriptor = descriptor->method(i); // 获取方法描述信息
        auto method_name = method_descriptor->name(); // 获取方法名称
        service_info.m_methodMap.insert({method_name, method_descriptor}); // 存储方法名称和方法描述信息的键值对
    }
    std::cout << "method size :" << method_count << std::endl;
    m_serviceInfoMap.insert({service_name, service_info}); // 将服务名称和服务信息的键值对插入到m_serviceInfoMap中
}

// 启动rpc服务发布节点，Run以后，进程进入阻塞状态，等待远程的rpc调用请求
void RpcProvider::Run()
{
    // 从配置文件中读取rpcserver的ip和port
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    // 创建一个InetAddress对象，表示rpcserver的地址
    muduo::net::InetAddress address(ip,port);
    // 创建一个TcpServer对象，绑定地址和事件循环
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");
    // 绑定连接回调和消息读写回调
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    server.setMessageCallback(
        std::bind(&RpcProvider::OnMessage,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3)
    );
    // 设置muduo的线程数量
    server.setThreadNum(4);
    std::cout << "RpcProvider start service at " << ip << ":" << port << std::endl;
    server.start(); // 启动服务器
    m_eventLoop.loop(); // 进入事件循环，等待客户端连接和请求 // Epoll.wait()
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if(!conn->connected())
    {
        // RPC和客户端连接断开
        conn->shutdown(); // 关闭连接
    }

}

// 将解析封装，解决TCP粘包与半包的问题
RpcProvider::ParseResult RpcProvider::parseRpcMessage(muduo::net::Buffer* buf, mprpc::RpcHeader* rpc_header, std::string& args_str)
{
    if(buf->readableBytes() < 4)
    {
        return ParseResult::kInComplete; // 数据不完整，等待下一次数据到来
    }
    uint32_t header_size = buf->peekInt32(); // 读取前4个字节，得到header_size
    std::cout << "DEBUG header_size: " << header_size
          << ", readable: " << buf->readableBytes() << std::endl;
    if(buf->readableBytes() < 4 + header_size)
    {
        return ParseResult::kInComplete; // 数据不完整，等待下一次数据到来
    }
    const char* ableReadDataPtr = buf->peek(); // 获取可读数据的指针
    std::string header_str = std::string(ableReadDataPtr + 4,header_size);// 从接收的数据中读取rpc_header_str
    std::string service_name;
    std::string method_name;
    uint32_t args_size = 0;
    if(rpc_header->ParseFromString(header_str))
    {
        service_name = rpc_header->service_name(); // 获取服务名称
        method_name = rpc_header->method_name(); // 获取方法名称
        args_size = rpc_header->args_size(); // 获取参数大小
    }
    else
    {
        std::cout << "rpc_header_str: " << header_str << " parse error!" << std::endl;
        return ParseResult::kError; // 解析错误
    }
    if(buf->readableBytes() < 4 + header_size + args_size)
    {
        return ParseResult::kInComplete; // 数据不完整，等待下一次数据到来
    }
    args_str = std::string(ableReadDataPtr + 4 + header_size,args_size); // 从接收的数据中读取args_str
    buf->retrieve(4 + header_size + args_size); // 从缓冲区中移除已经读取的数据
    return ParseResult::kComplete; // 解析完成
}

void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, 
    muduo::net::Buffer* buf, 
    muduo::Timestamp timestamp)
{
    /*
        TCP => header_size(4) + header_str + args_str
        header_str => service_name method_name args_size
        1. 通过 string::copy()读取前四个字节获取header_size
        2. 根据 header_size 读取 rpc_header_str
        3. 反序列化 rpc_header_str，获取 service_name method_name args_size
        4. 根据 args_size 读取 args_str

    std::string recv_buf = buf->retrieveAllAsString(); 
    // 接收数据 ？ 接收的只是缓冲区的全部数据，能否完整的包含一个RPC请求的数据？ 可能会有粘包和半包问题

    uint32_t header_size_net = 0;
    recv_buf.copy((char*)&header_size_net,4,0); // 从接收的数据中读取前4个字节，得到header_size
    uint32_t header_size = ntohl(header_size_net); // 将header_size转换为主机字节序
    std::string rpc_header_str = recv_buf.substr(4,header_size); // 从接收的数据中读取rpc_header_str
    mprpc::RpcHeader rpc_header;
    // 反序列化rpc_header_str，得到rpc_header对象
    std::string service_name;
    std::string method_name;
    uint32_t args_size = 0;

    if(rpc_header.ParseFromString(rpc_header_str))
    {
        service_name = rpc_header.service_name(); // 获取服务名称
        method_name = rpc_header.method_name(); // 获取方法名称
        args_size = rpc_header.args_size(); // 获取参数大小
    }
    else
    {
        std::cout << "rpc_header_str: " << rpc_header_str << " parse error!" << std::endl;
        return;
    }

    std::string args_str = recv_buf.substr(4 + header_size,args_size);

    // 打印调试信息
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_size: " << args_size << std::endl;

    */
    // 解析请求数据
    mprpc::RpcHeader rpc_header;
    std::string args_str;
    while(buf -> readableBytes() > 0)
    {
        ParseResult result = parseRpcMessage(buf, &rpc_header, args_str);
        if(result == ParseResult::kInComplete)
        {
            std::cout << "recv data is incomplete, wait for next recv data!" << std::endl;
            break; // 数据不完整，等待下一次数据到来
        }
        else if(result == ParseResult::kError)
        {
            std::cout << "recv data is error!" << std::endl;
            conn -> shutdown(); // 解析错误，关闭连接
            return; // 解析错误
        }
        //获取service对象和method对象
        std::string service_name = rpc_header.service_name(); // 获取服务名称
        std::string method_name = rpc_header.method_name(); // 获取方法名称
        auto service_it = m_serviceInfoMap.find(service_name);
        if(service_it == m_serviceInfoMap.end())
        {
            std::cerr << "service: " << service_name << " is not exist!" << std::endl;
            conn -> shutdown(); // 服务不存在，关闭连接
            return;
        }
        google::protobuf::Service* service = service_it -> second.service; // 获取服务对象
        auto method_it = service_it -> second.m_methodMap.find(method_name);
        if(method_it == service_it -> second.m_methodMap.end())
        {
            std::cerr << service_name << "method: " << method_name << " is not exist!" << std::endl;
            conn -> shutdown(); // 方法不存在，关闭连接
            return;
        }
        const google::protobuf::MethodDescriptor* method = method_it -> second; // 获取方法

        google::protobuf::Message* request = service -> GetRequestPrototype(method).New();

        if(!request -> ParseFromString(args_str))
        {
            std::cerr << "request parse error!" << std::endl;
            return;
        }

        google::protobuf::Message* response = service -> GetResponsePrototype(method).New();
        // 绑定回调函数，发送rpc响应
        google::protobuf::Closure * done = google::protobuf::NewCallback<
        RpcProvider,
        const muduo::net::TcpConnectionPtr&, 
        google::protobuf::Message*>
        (
            this, &RpcProvider::sendRpcResponse, conn, response
        ); 
        // 在框架上进行一次rpc方法调用
        service -> CallMethod(method,nullptr,request,response,done);
    }
}

void RpcProvider::sendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response)
{
    // 响应序列化
    std::string response_str;
    if(response->SerializeToString(&response_str))
    {
        uint32_t response_size = response_str.size();
        uint32_t response_size_net = htonl(response_size); // 将response_size转换
        std::string send_buf = std::string((char*)&response_size_net,4) + response_str; // response_size + response_str
        conn -> send(send_buf); // 通过连接发送响应数据
        delete response; // 释放响应对象的内存
    }
    else
    {
        std::cerr << "response serialize error!" << std::endl;
        delete response; // 释放响应对象的内存
    }
    // 采用长连接
    // conn -> shutdown(); // 模拟Http的短链接服务，由rpcprovider主动断开连接
}