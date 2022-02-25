#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
#include <ctime>

using namespace std;

//数据库操作类
class MysqlConn
{
public:
    //初始化数据库连接
    MysqlConn();
    //释放数据库连接资源
    ~MysqlConn();
    //连接数据库
    bool connect(string ip, unsigned short port, string user, string password, string dbname);
    //更新操作
    bool update(string sql);
    //查询操作
    MYSQL_RES *query(string sql);
    
    //刷新一下连接的起始的空闲时间
    void refreshAliveTime() { _alivetime = clock(); }
    //返回存活的时间
    clock_t getAliveTime() const { return clock() - _alivetime; }

    //获取连接
    MYSQL* getConnection();
private:
    MYSQL* _conn;   // 表示和MYSQL Server的一条连接
    clock_t _alivetime; // 记录进入空闲状态后的起始存活时间
};

#endif