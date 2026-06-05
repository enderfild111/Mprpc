#include "mprpcchannel.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include <arpa/inet.h> // for htonl and ntohl
#include <sys/socket.h>
#include <netinet/in.h>
#include <error.h>
#include <unistd.h>
#include "managefd.h"
#include "mprpczookeeper.h"
#include "mprpcLog.h"
/*
    header_size + header_str(service_name method_name args_size) + args_str
    header_size: 4字节，存储rpc_header_str的大小

*/
bool MprpcChannel::read_n(int fd, void* buf, int n)
{
    size_t nleft = n;
    char* ptr = static_cast<char*>(buf);
    while(nleft > 0)
    {
        ssize_t nread = recv(fd,ptr,nleft,0);
        if (nread <= 0) return false;   // 关闭或出错
        nleft -= nread;
        ptr += nread;
    }
    return true;
}

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name(); // 获取服务名称
    std::string method_name = method->name(); // 获取方法名称
    // 参数的序列化
    uint32_t args_size = 0;
    std::string args_str;
    if(!request -> SerializeToString(&args_str))
    {
        LOG(ERROR) << "Failed to serialize request!";
        controller->SetFailed("Failed to serialize request!");
        return;
    }
    else
    {
        args_size = args_str.size();
    }
    mprpc::RpcHeader rpcHeader;

    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_str.size());

    std::string rpc_header_str;
    uint32_t header_size = 0;
    if(!rpcHeader.SerializeToString(&rpc_header_str))
    {
        LOG(ERROR) << "Failed to serialize rpc header!";
        controller->SetFailed("Failed to serialize rpc header!");
        return;
    }
    else
    {
        header_size = rpc_header_str.size();
    }
 
    std::string send_buf;
    uint32_t header_size_net = htonl(header_size); // 将header_size转换为网络字节序
    send_buf.append(std::string((char*)&header_size_net,4)).append(rpc_header_str).append(args_str); // header_size
    /*
        std::cout << "header_size: " << header_size 
          << ", rpc_header_str: " << rpc_header_str 
          << ", args_str: " << args_str 
          << ", send_buf.size(): " << send_buf.size() << std::endl;
    */

    // Tcp 编程完成远程调用的发送
    if(done == nullptr)
    {
        /*
            用智能指针管理socketfd?，socketfd，用unique_ptr 放在堆上并不必要，
            这里使用unique_fd进行管理
            直接放在栈上，unique_fd的析构函数会自动调用close函数关闭socketfd
        */
        unique_fd clientfd(socket(AF_INET,SOCK_STREAM,0));
        // decltype(&close) -> int (*)(int)
        // 同步
        if(!clientfd)
        {
            std::string errMsg = "create socket error! errno: "+ std::to_string(errno);
            LOG(ERROR) << errMsg;
            controller->SetFailed(errMsg);
            return;
        }

        struct sockaddr_in server_addr;
        // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
        // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
        ZookeeperClient zkc;
        zkc.Start();
        std::string method_path = "/" + service_name + "/" + method_name;
        std::string host_data = zkc.GetData(method_path.c_str());
        if(host_data == "")
        {
            LOG(ERROR) << method_path << "is not exist";
            controller->SetFailed(method_path + "is not exist");
            return;
        }
        auto idx = host_data.find(":");
        if(idx == -1)
        {
            LOG(ERROR) << method_path << "address is invalid";
            controller->SetFailed(method_path + "address is invalid");
            return;
        }
        std::string ip = host_data.substr(0,idx);
        uint16_t port = atoi(host_data.substr(idx + 1,host_data.size() - idx).c_str());
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        if( -1 == connect(clientfd.get(),(struct sockaddr*)&server_addr,sizeof(server_addr)))
        {
            std::string errMsg = "connect error! errno: "+ std::to_string(errno);
            LOG(ERROR) << errMsg;
            controller->SetFailed(errMsg);
            return;
        }
        if(-1 == send(clientfd.get(),send_buf.c_str(),send_buf.size(),0))
        {
            std::string errMsg = "send error! errno: "+ std::to_string(errno);
            LOG(ERROR) << errMsg;
            controller->SetFailed(errMsg);
            return;
        }
        /*
        // 接收Rpc响应
        std::string response_str;
        char buf[4096];                       // 栈缓冲区，大小可调
        ssize_t n;
        while ((n = recv(clientfd.get(), buf, sizeof(buf), 0)) > 0) {
            response_str.append(buf, n);
        }
        if(n == -1)
        {
            std::cerr << "recv error! errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }
        if(response->ParseFromString(response_str))
        {
            std::cout << "response_str: " << response_str << std::endl;
        }
        else
        {
            std::cerr << "Failed to parse response!" << std::endl;
            return;
        } 
        */
        // 长连接情况下接收Rpc响应
        uint32_t response_size_net = 0;
        if(!read_n(clientfd.get(), &response_size_net, 4))
        {
            LOG(ERROR) << "Failed to read response size!";
            controller->SetFailed("Failed to read response size!");
            return;
        }
        uint32_t response_size = ntohl(response_size_net); // 将response_size转换为主机字节序
        std::string response_str(response_size, '\0'); // 创建一个大小为response_size
        if(!read_n(clientfd.get(), &response_str[0], response_size))
        {
            LOG(ERROR) << "Failed to read response data!";
            controller->SetFailed("Failed to read response data!");
            return;
        }
        if(response->ParseFromString(response_str))
        {
            LOG(DEBUG) << "response_str: " << response_str;
            // std::cout << "response_str: " << response_str << std::endl; // 调试用
        }
        else
        {
            LOG(ERROR) << "Failed to parse response!";
            controller->SetFailed("Failed to parse response!");
            return;
        }
    }
    else
    {
        
    }
}