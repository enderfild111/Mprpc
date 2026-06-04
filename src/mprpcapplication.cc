#include "mprpcapplication.h"
#include <unistd.h>
#include <iostream>
#include <string>

void ShowArgcHelp()
{
    std::cout << "format: command -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char *argv[])
{
    // 框架的初始化操作
    if(argc < 2)
    {
        ShowArgcHelp();
        exit(EXIT_FAILURE);
    }
    // 解析命令行参数，加载配置文件等等
    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1)
    {
        switch(c)
        {
            case 'i':
                config_file = optarg; // 获取配置文件路径
                break;
            case '?':
                ShowArgcHelp();
                exit(EXIT_FAILURE);
            case ':':
                ShowArgcHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
        // c = getopt(argc, argv, "i:");
    }
    // 加载配置文件 rpcserver_ip=? rpcserver_port=? zookeeper_ip=? zookeeper_port=?
    auto &config = MprpcApplication::GetInstance().GetConfig();
    // std::cout << "config_file: " << config_file << std::endl;
    config.LoadConfigFile(config_file);
    /*
    测试
    std::cout << "rpcserver_ip: " << config.Load("rpcserverip") << std::endl;
    std::cout << "rpcserver_port: " << config.Load("rpcserverport") << std::endl;
    std::cout << "zookeeper_ip: " << config.Load("zookeeperip") << std::endl;
    std::cout << "zookeeper_port: " << config.Load("zookeeperport") << std::endl;
    */
}

MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

Config& MprpcApplication::GetConfig()
{
    return m_config;
}