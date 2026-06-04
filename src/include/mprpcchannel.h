#pragma once
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <iostream>
class MprpcChannel : public google::protobuf::RpcChannel
{
public:
    // *Service_stub->RpcMethod() => MprpcChannel->CallMethod(),所有的Rpc方法都走CallMethod，这里要做序列化与反序列化
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done) override;
    static bool read_n(int fd, void* buf, int n);            
};