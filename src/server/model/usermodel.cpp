#include <usermodel.hpp>
#include "db.h"
#include <iostream>
#include <string>
using namespace std;

//User表的增加方法
bool UserModel::insert(User &user)
{
    //1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name,password,state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            //获取插入成功的用户数据生成的主键id  ???
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}

User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
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
    }
    //mysql连接失败，返回一个默认构造的User，id默认为-1
    return User();
}

//更新用户的状态信息
bool UserModel::updateState(User user)
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

//重置用户的状态信息
void UserModel::resetState()
{
    char sql[1024] = "update user set state = 'offline' where state = 'online' ";

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}