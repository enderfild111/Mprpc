#include "../src/include/mprpczookeeper.h"
#include "mprpcapplication.h"
int main()
{
    MprpcApplication::GetInstance().GetConfig().LoadConfigFile("/home/hpz/code/cpp/mprpc/bin/test.conf");
    ZookeeperClient zkc;
    zkc.Start();
    char data[10] = "mynd";
    zkc.Create("/myzknode",data,sizeof(data),0);
    return 0;
}