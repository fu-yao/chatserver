#ifndef USERMODEL_H
#define USERMODEL_H

#include <user.hpp>

class UserModel
{
public:
    //User表的增加方法
    bool insert(User& user);
    //根据用户号码查询用户查询
    User query(int id);
    //更新用户的状态信息
    bool updateState(User user);
    //重置用户的状态信息
    void resetState();
};



#endif