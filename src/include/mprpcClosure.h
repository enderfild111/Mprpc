#pragma once
#include <google/protobuf/stubs/callback.h>
#include <google/protobuf/message.h>

class MprpcClosure : public google::protobuf::Closure
{
public:
    MprpcClosure(google::protobuf::Closure* done, google::protobuf::Message* response)
    : done_(done)
    , response_(response)
    {}
    void Run() override
    {
        if(done_)
        {
            done_->Run();
            delete done_;
        }
        delete response_;
        delete this;
    }
private:
    google::protobuf::Closure* done_;
    google::protobuf::Message* response_;
};

