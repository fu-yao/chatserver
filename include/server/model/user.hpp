#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

//  User表的ORM类 object relation map对象关系映射表
class User
{
public:
    User(int id=-1,string name="",string pwd="",string state="offline")
        :   _id(id),_name(name),_password(pwd),_state(state)
    {

    }

    void setId(int id) { this->_id = id; }
    void setName(string name) { this->_name = name; }
    void setPwd(string pwd) { this->_password = pwd; }
    void setState(string state) { this->_state = state; }

    int getId() { return _id; }
    string getName() { return _name; }
    string getPwd() { return _password; }
    string getState() { return _state; }
private:
    int _id;
    string _name;
    string _password;
    string _state;
};


#endif