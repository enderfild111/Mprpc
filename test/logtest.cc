#include <string>
#include <ctime>
#include <chrono>
#include <iostream>
#include "../src/include/mprpcLog.h"

int main() {
    
    MprpcLogger::getInstance().set_minLogLevel(LogLevel::INFO);
    LOG(DEBUG) << "RPC 框架初始化成功";
    LOG(INFO) << "开始监听端口 8888";
    LOG(WARNING) << "配置文件不存在，使用默认配置";
    LOG(ERROR) << "连接数据库失败";

    // 让日志有时间输出
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}