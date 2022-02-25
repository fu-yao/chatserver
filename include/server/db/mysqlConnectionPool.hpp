#pragma once
#include <mymuduo/Logger.hpp>
#include <string>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "db.hpp"


using namespace std;

class MysqlConnectionPool
{
public:
    //获取连接池对象实例
    static MysqlConnectionPool* getMysqlConnectionPool();
    //提供给外部的接口,从连接池中获取一个可用的空闲连接
    shared_ptr<MysqlConn> getMysqlConnection();

    //这些东西放置在配置文件中
    string ip_;             //mysql的ip地址
    unsigned short port_;   //mysql的端口号 默认3306
    string username_;       //mysql的登录用户名
    string password_;       //mysql的登录密码
    string dbname_;         //连接数据库的名称
    int initSize_;          //连接池初始连接量
    int maxSize_;           //连接池最大连接量
    int maxIdleTime_;       //连接池最大空闲时间
    int connectionTimeOut_; //连接池获取连接的超时时间

private:
    //单例  构造函数私有化
    MysqlConnectionPool();

    //加载配置文件
    bool loadConfigFile();

    //运行在独立的线程中，专门负责生成新连接
    void produceConncetionTask();

    //扫描超过maxIdleTime时间的空闲连接，进行对应的连接回收
    void scannerConnectionTask();

    queue<MysqlConn*> mysqlConnectionQue;   //存储mysql连接的队列
    mutex queueMutex;   //维护连接队列的线程安全互斥锁
    atomic_int mysqlConnectionCnt;  //记录连接所创建的connnection连接的总数量
    condition_variable cv;  //设置条件变量，用于连接生产进程和连接消费进程的通信
};


