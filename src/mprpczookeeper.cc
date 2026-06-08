#include <mprpczookeeper.h>
#include <semaphore>
#include "mprpcapplication.h"
#include "mprpcLog.h"

// zkserver 给 zkclient的通知回调函数
void Mywatcher(zhandle_t *zh, int type,int state, const char *path,void *watcherCtx)
{
    if(type == ZOO_SESSION_EVENT) //回调消息类型是会话型消息
    {
        if(state == ZOO_CONNECTED_STATE) // zkclient 和 zkserver 连接成功
        {
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}


ZookeeperClient::ZookeeperClient()
    : zkhandle_(nullptr)
{

}

ZookeeperClient::~ZookeeperClient()
{
    if(zkhandle_ != nullptr)
    {
        zookeeper_close(zkhandle_);
    }
}

void ZookeeperClient::Start()
{
    std::string zkip = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string zkport = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string host = zkip + ":" + zkport;

    zkhandle_ = zookeeper_init(&host[0],Mywatcher,30000,nullptr,nullptr,0);
    if(zkhandle_ == nullptr)
    {
        LOG(ERROR) << "zk init error";
        exit(EXIT_FAILURE);
    }
    sem_t sem;
    sem_init(&sem,0,0);
    zoo_set_context(zkhandle_,&sem);
    sem_wait(&sem);
    LOG(INFO) << "zk init success";    
}

void ZookeeperClient::Create(const char* path,const char* data,int datalen,int state)
{
    int flag = 0;
    char buf[512];
    if((flag = zoo_exists(zkhandle_,path,0,nullptr)) == ZNONODE)
    {
        flag = zoo_create(zkhandle_,path,data,datalen,&ZOO_OPEN_ACL_UNSAFE,state,buf,sizeof(buf));
        if(flag == ZOK)
        {
            LOG(INFO) << "create zk node success, path: " << path;
        }
        else
        {
            LOG(INFO) << "create zk node faild, path: " << path << "flag: " << flag;
            exit(EXIT_FAILURE);
        }
    }
}

std::string ZookeeperClient::GetData(const char* path)
{
    char buf[128];
    int buflen = sizeof(buf);
    
    int flag = zoo_get(zkhandle_,path,0,buf,&buflen,nullptr);

    if(flag == ZOK)
    {
        return buf;
    }
    else
    {
        LOG(ERROR) << "get data faild in path: " << path;
        return "";
    }
}

