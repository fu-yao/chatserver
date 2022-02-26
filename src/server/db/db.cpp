#include "db.hpp"
#include <mymuduo/Logger.hpp>


//  数据库配置信息
static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "123456";
static std::string dbname = "chat";

//初始化数据库连接
MysqlConn::MysqlConn()
{
    _conn = mysql_init(nullptr);
}

//释放数据库连接资源
MysqlConn::~MysqlConn()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}
//连接数据库
bool MysqlConn::connect(string i, unsigned short port, string user, string password, string dbname)
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
    if (p != nullptr)
    {
        // C和C++代码默认的编码字符是ASCII，若不设置，从MySQL上拉下来的中文显示乱码
        mysql_query(_conn, "set name gbk!");
        //LOG_INFO("connect mysql success!");
    }
    else
    {
        //LOG_INFO("connect mysql fail!");
        return false;
    }

    return true;
}
//更新操作
bool MysqlConn::update(string sql)
{
    
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO("%s:%s:%s 更新失败!",__FILE__,__LINE__,sql);
        return false;
    }
    //LOG_INFO("%s:%s:%s 更新成功!",__FILE__,__LINE__,sql);
    return true;
}
//查询操作
MYSQL_RES* MysqlConn::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO("%s:%s:%s 查询失败",__FILE__, __LINE__, sql);
        return nullptr;
    }
    return mysql_use_result(_conn);
}

MYSQL* MysqlConn::getConnection()
{
    return _conn;
}
