#pragma once
#include <string>
#include "unablecopy.h"
#include <unordered_map>
// 配置加载类，负责加载配置文件，单例模式
class Config : UnableCopy
{
    public:
    // 加载配置文件
    void LoadConfigFile(const std::string& config_file);
    Config(){};
    // 根据key获取对应的value值
    std::string Load(const std::string& key);
    private:
    // 去掉字符串前后空格
    void Trim(std::string& src_buf);
    std::unordered_map<std::string, std::string> configMap; // 存储配置项的键值对
};