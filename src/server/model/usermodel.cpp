#include "db.hpp"
#include "mysqlConnectionPool.hpp"

#include <usermodel.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <sys/time.h>
using namespace std;

//测试连接池性能
// bool UserModel::insert(User &user)
// {
//     struct timeval tv1,tv2;
//     struct timezone tz1,tz2;

//     gettimeofday(&tv1,&tz1);

//     // thread t1([&]() {
//     //     MysqlConnectionPool* cp = MysqlConnectionPool::getMysqlConnectionPool();
//     //     for(int i=0; i<250; ++i)
//     //     {
//     //         //组装sql语句
//     //         char sql[1024] = {0};
//     //         sprintf(sql, "insert into test(name,password) values('%s','%s')",
//     //                 user.getName().c_str(), user.getPwd().c_str());

//     //         shared_ptr<MysqlConn> mysql = cp->getMysqlConnection();
//     //         mysql->update(sql);
//     //     }

        
//     //     // for(int i=0; i<2500; ++i)
//     //     // {
//     //     //     MysqlConn mysql;
//     //     //     //组装sql语句
//     //     //     char sql[1024] = {0};
//     //     //     sprintf(sql, "insert into test(name,password) values('%s','%s')",
//     //     //             user.getName().c_str(), user.getPwd().c_str());

//     //     //     mysql.connect("127.0.0.1", 3306, "root", "123456!", "chat");
//     //     //     mysql.update(sql);
//     //     // }

//     // });

    
//     // MysqlConnectionPool* cp = MysqlConnectionPool::getMysqlConnectionPool();
//     // for(int i=0; i<5000; ++i)
//     // {
//     //     //组装sql语句
//     //     char sql[1024] = {0};
//     //     sprintf(sql, "insert into test(name,password) values('%s','%s')",
//     //             user.getName().c_str(), user.getPwd().c_str());

//     //     shared_ptr<MysqlConn> mysql = cp->getMysqlConnection();
//     //     mysql->update(sql);
//     // }

    
//     for(int i=0; i<5000; ++i)
//     {
//         MysqlConn mysql;
//         //组装sql语句
//         char sql[1024] = {0};
//         sprintf(sql, "insert into test(name,password) values('%s','%s')",
//                 user.getName().c_str(), user.getPwd().c_str());

//         mysql.connect("127.0.0.1", 3306, "root", "123456!", "chat");
//         mysql.update(sql);
//     }

//     gettimeofday(&tv2,&tz2);
//     cout << tv2.tv_sec-tv1.tv_sec << "s" << endl;   //豪秒
//     cout << (tv2.tv_sec*1000 + tv2.tv_usec/1000)-(tv1.tv_sec*1000 + tv1.tv_usec/1000) << "ms" << endl;   //豪秒
    
//     return false;
// }



//User表的增加方法
bool UserModel::insert(User &user)
{
    MysqlConnectionPool* cp = MysqlConnectionPool::getMysqlConnectionPool();
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name,password,state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    shared_ptr<MysqlConn> mysql = cp->getMysqlConnection();

    if (mysql->update(sql))
    {
        //获取插入成功的用户数据生成的主键id  ???
        user.setId(mysql_insert_id(mysql->getConnection()));
        return true;
    }


    return false;
}

User UserModel::query(int id)
{
    MysqlConnectionPool* cp = MysqlConnectionPool::getMysqlConnectionPool();
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);

    shared_ptr<MysqlConn> mysql = cp->getMysqlConnection();

    MYSQL_RES *res = mysql->query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row != nullptr)
        {
            User user;
            //这里0 1 2 3对应数据库chat中user表的顺序
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setPwd(row[2]);
            user.setState(row[3]);
            //记得释放指针
            mysql_free_result(res);
            return user;
        }
    }

    //mysql连接失败，返回一个默认构造的User，id默认为-1
    return User();
}

//更新用户的状态信息
bool UserModel::updateState(User user)
{
    MysqlConnectionPool* cp = MysqlConnectionPool::getMysqlConnectionPool();
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    shared_ptr<MysqlConn> mysql = cp->getMysqlConnection();

    if (mysql->update(sql))
    {
        return true;
    }

    return false;
}

//重置用户的状态信息
void UserModel::resetState()
{
    MysqlConnectionPool* cp = MysqlConnectionPool::getMysqlConnectionPool();
    char sql[1024] = "update user set state = 'offline' where state = 'online' ";

    shared_ptr<MysqlConn> mysql = cp->getMysqlConnection();

    mysql->update(sql);

}