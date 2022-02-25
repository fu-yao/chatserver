#include "mysqlConnectionPool.hpp"

#include <thread>
#include <iostream>
#include <functional>

//获取连接池对象实例
MysqlConnectionPool* MysqlConnectionPool::getMysqlConnectionPool()
{
    static MysqlConnectionPool pool;    // lock 和 unlock
    return &pool;
}


//提供给外部的接口,从连接池中获取一个可用的空闲连接
shared_ptr<MysqlConn> MysqlConnectionPool::getMysqlConnection()
{
    unique_lock<mutex> lock(queueMutex);
    while(mysqlConnectionQue.empty())
    {
        //不能写sleep,sleep就直接睡了，这里是一直监听
        //醒来仍然是空的，要么别人把我唤醒，要么超时
        if(cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(connectionTimeOut_)))
        {
            if(mysqlConnectionQue.empty())
            {
                LOG_INFO("获取空闲连接超时了...获取连接失败！");
                return nullptr;
            }
        }
    }

    //如果队列不为空
    //这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
    //所以加上互斥锁
    //刷新一下空闲的存活时间
    shared_ptr<MysqlConn> sp(mysqlConnectionQue.front(),
        [&](MysqlConn* pcon){
            unique_lock<mutex> lock(queueMutex);
            mysqlConnectionQue.push(pcon);
        }
    );

    mysqlConnectionQue.pop();
    cv.notify_all();

    return sp;
}


//单例  构造函数私有化
MysqlConnectionPool::MysqlConnectionPool()
{
    //加载配置项
    if(!loadConfigFile())
    {
        LOG_ERROR("mysql.conf load error!");
        return ;
    }

    //创建初始数量的连接
    for(int i=0; i<initSize_; ++i)
    {
        MysqlConn* p = new MysqlConn();
        p->connect(ip_, port_, username_, password_, dbname_);
        p->refreshAliveTime();  //刷新一下空闲的存活时间
        mysqlConnectionQue.push(p);
        mysqlConnectionCnt++;
    }

    //启动一个新的线程，作为生产者  
    thread produce(std::bind(&MysqlConnectionPool::produceConncetionTask,this));
    //设置成守护线程，分离线程  =》 线程完了，守护线程自动结束
    produce.detach();

    //启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行对应的连接收回
    thread scanner(std::bind(&MysqlConnectionPool::scannerConnectionTask,this));
    scanner.detach();


}

//加载配置文件
bool MysqlConnectionPool::loadConfigFile()
{
    FILE* pf = fopen("/root/myProject/ChatRoom/src/server/db/mysql.conf","r");
	if (pf == nullptr)
	{
		LOG_INFO("mysql.txt file is not exist!");
		return false;
	}

	while (!feof(pf))
	{
		char line[1024] = { 0 };
		fgets(line,1024,pf);
		string str = line;
		//cout << line << endl;
		int idx = str.find('=',0);
		if (idx == -1)//无效的配置项
		{
			continue;
		}

		//password=123456\n
		int endidx = str.find('\n',idx);
		string key = str.substr(0,idx);
		string value = str.substr(idx + 1, endidx - idx -1);
		
		if(key == "ip")
		{
			ip_ =value;
		}else if(key == "port"){
			port_ = atoi(value.c_str());
		}
		else if (key == "username") {
			username_ = value;
		}
		else if (key == "password") {
			password_ = value;
		}
		else if (key == "dbname") {
			dbname_ = value;
		}
		else if (key == "initSize") {
			initSize_ = atoi(value.c_str());
		}
		else if (key == "maxSize") {
			maxSize_ = atoi(value.c_str());
		}
		else if (key == "maxIdleTime") {
			maxIdleTime_ = atoi(value.c_str());
		}
		else if (key == "connectionTimeOut") {
			connectionTimeOut_ = atoi(value.c_str());
		}
	}
	return true;
}

//运行在独立的线程中，专门负责生成新连接
void MysqlConnectionPool::produceConncetionTask()
{
    for(;;)
    {
        unique_lock<mutex> lock(queueMutex);
        while(!mysqlConnectionQue.empty())
        {
            cv.wait(lock);  //队列不空，此处生产线程进入等待状态
        }

        //连接数量没有达到上限，继续创建新的连接
        if(mysqlConnectionCnt < maxSize_)
        {
            MysqlConn* p = new MysqlConn();
            p->connect(ip_, port_, username_, password_, dbname_);
            p->refreshAliveTime();  //刷新一下空闲的存活时间
            mysqlConnectionQue.push(p);
            mysqlConnectionCnt++;
        }

        //通知消费者线程，可以消费连接了
        cv.notify_all();
    }
}

//扫描超过maxIdleTime时间的空闲连接，进行对应的连接回收
void MysqlConnectionPool::scannerConnectionTask()
{
    //扫描整个队列，释放多余的连接
    for(;;)
    {
        //通过sleep模拟定时效果
        this_thread::sleep_for(chrono::seconds(maxIdleTime_));
        unique_lock<mutex> lock(queueMutex);
        while(mysqlConnectionCnt > initSize_)
        {
            MysqlConn* p = mysqlConnectionQue.front();
            if(p->getAliveTime() >= (maxIdleTime_ * 1000))
            {
                mysqlConnectionQue.pop();
                mysqlConnectionCnt--;
                delete p;   //调用~MysqlConn()释放连接
            }
            else{
                break;  //队头连接都没超过maxIdelTime，其他也不会超过
            }
        }
    }
}