#include "friendmodel.hpp"
#include "db.hpp"
#include "mysqlConnectionPool.hpp"

//添加好友关系
void FriendModel::insert(int userid,int friendid)
{
    MysqlConnectionPool* cp = MysqlConnectionPool::getMysqlConnectionPool();
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"insert into friend values(%d,%d)",userid,friendid);

    shared_ptr<MysqlConn> mysql = cp->getMysqlConnection();
    
    mysql->update(sql);
}

//返回用户好友列表，此处直接做联合查询，而不是分开两次查
vector<User> FriendModel::query(int userid)
{
    MysqlConnectionPool* cp = MysqlConnectionPool::getMysqlConnectionPool();
    //组装sql语句
    char sql[1024] = {0};
    
    sprintf(sql,"select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid = %d",userid);

    vector<User> vec;
    shared_ptr<MysqlConn> mysql = cp->getMysqlConnection();
    MYSQL_RES* res = mysql->query(sql);
    if(res != nullptr)
    {
        //
        MYSQL_ROW row;
        while( (row = mysql_fetch_row(res)) != nullptr)
        {
            User user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setState(row[2]);
            vec.push_back(user);
        }
        mysql_free_result(res);
        return vec;
    }        
    //有考虑到空的情况
    return vec;
 
}