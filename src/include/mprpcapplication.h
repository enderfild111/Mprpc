#pragma once
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
// 单例模式
class MprpcApplication
{
public:
    // 负责框架的初始化操作
    static void Init(int argc, char* argv[]);
    static MprpcApplication& GetInstance();
    Config& GetConfig();
private:
    Config m_config; // 配置对象
    MprpcApplication() {}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication& operator=(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
    MprpcApplication& operator=(MprpcApplication&&) = delete;
};