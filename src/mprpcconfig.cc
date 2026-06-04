#include "mprpcconfig.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "mprpcLog.h"
void Config::LoadConfigFile(const std::string& config_file)
{
    // 加载配置文件
    std::ifstream file(config_file);
    if(!file.is_open())
    {
        LOG(ERROR) << "Failed to open config file: " << config_file;
        std::cerr << "Failed to open config file: " << config_file << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string line;
    while(std::getline(file,line))
    {
        if(line.empty() || line[0] == '#') // 跳过空行和注释行
        {
            continue;
        }
        
        else
        {
            Trim(line); // 去掉字符串前后空格
            std::string key, value;
            std::istringstream iss(line);
            if(std::getline(iss, key, '=') && std::getline(iss, value))
            {
                Trim(key); // 去掉字符串前后空格
                Trim(value); // 去掉字符串前后空格
                configMap[key] = value; // 存储配置项的键值对
            }
        }
    }
    return;
}

std::string Config::Load(const std::string& key)
{
    // 从加载的配置文件中根据key获取对应的value值
    auto it = configMap.find(key);
    if(it != configMap.end())
    {
        return it->second;
    }
    else
    {
        LOG(ERROR) << "Key not found in config: " << key;
        std::cerr << "Key not found in config: " << key << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Config::Trim(std::string& src_buf)
{
    src_buf.erase(0,src_buf.find_first_not_of(" \t\r\n")); // 去掉字符串前面的空格
    src_buf.erase(src_buf.find_last_not_of(" \t\r\n") + 1); // 去掉字符串后面的空格
}