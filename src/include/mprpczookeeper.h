#pragma once
#include <zookeeper/zookeeper.h>
#include <string>

class ZookeeperClient
{
public:
    ZookeeperClient();
    ~ZookeeperClient();
    // 启动连接ZKserver
    void Start();
    // 在server上根据路径创建node
    void Create(const char* path,const char* data,int datalen,int state = 0);
    std::string GetData(const char* path);

private:
    // zk 客户端句柄
    zhandle_t* zkhandle_;
};

