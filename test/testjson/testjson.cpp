#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
using namespace std;
#include <map>
#include <vector>
#include <string>

/*json的使用与无序容器非常像，底层原理是链式哈希*/

/*
    千万注意c_str()的用法
    const char* c_str()
    函数返回的是const char*类型
    不可以直接将字符串转换给const char 指针
    因为s对象会析构，最后c指向的是一个垃圾

    要用strcpy处理
    char c*;   会报错，没分配内存
    char c[20];
    strcpy(c,s.c_str());
*/
void cstr()
{
    
    string s = "123456";
    char c[20];
    //c = s.c_str();
    strcpy(c,s.c_str());
    cout << c <<endl;
}

//序列化示例1
void func1()
{
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhangsan";
    js["to"] = "lisi";
    js["msg"] = "How old are you ?";
    //cout << js << endl;

    //dump是输出的意思，就是将js转换成字符串
    string stringbuf = js.dump();
    cout << stringbuf.c_str() << endl;
}

//序列化示例2
void func2()
{
    json js;
    //
    js["id"] = {1,2,3,4,5,6,7};
    js["msg"]["name"] = "zhang san";
    js["msg"]["country"] = "China";

    //等价于上述写法，由于键值对不能重复，就覆盖了/没加进去
    //大括号括起来 整个都是值
    //值里面 又有 键值对
    js["msg"] = {{"name","zhang san"},{"country","China"}};

    //dump是输出的意思，就是将js转换成字符串
    string stringbuf = js.dump();
    cout << stringbuf.c_str() << endl;
}

//序列化示例3
void func3()
{
    json js;

    //js里面可以装数组(比较少用到)
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    js["list"] = vec;

    map<int,string> m;
    m.insert({1,"黄山"});
    m.insert({2,"华山"});
    m.insert({3,"泰山山"});
    js["path"] = m;

    //cout << js << endl;
    string sendBuf = js.dump(); //json数据对象 => 序列化 => json字符串
    //cout << sendBuf << endl;
   
    char strBuf[1024];
    strcpy(strBuf,sendBuf.c_str());
    cout << strBuf << endl;

}

//反序列化示例1
string func11()
{
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhangsan";
    js["to"] = "lisi";
    js["msg"] = "How old are you ?";
    //cout << js << endl;

    //dump是输出的意思，就是将js转换成字符串
    string strBuf = js.dump();
    return strBuf;
}
//反序列化示例2
string func22()
{
    json js;
    //
    js["id"] = {1,2,3,4,5,6,7};
    js["msg"]["name"] = "zhang san";
    js["msg"]["country"] = "China";

    //等价于上述写法，由于键值对不能重复，就覆盖了/没加进去
    //大括号括起来 整个都是值
    //值里面 又有 键值对
    js["msg"] = {{"name","zhang san"},{"country","China"}};

    return js.dump();
}

//反序列化示例3
string func33()
{
    json js;

    //js里面可以装数组(比较少用到)
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    js["list"] = vec;

    map<int,string> m;
    m.insert({1,"黄山"});
    m.insert({2,"华山"});
    m.insert({3,"泰山山"});
    js["path"] = m;

    //cout << js << endl;
    string sendBuf = js.dump(); //json数据对象 => 序列化 => json字符串
    //cout << sendBuf << endl;
   
    return sendBuf;
}

int main()
{ 
    //测试序列化
    //func3();
    
    //测试反序列化
    string recBuf = func33();
    json jsBuf = json::parse(recBuf);
    //保存里数据类型，如msg_type是整型，其余字符串
    // cout << jsBuf << endl;
    // cout << jsBuf["from"] << endl;
    // cout << jsBuf["to"] << endl;
    // cout << jsBuf["msg_type"] << endl;
    // cout << jsBuf["msg"] << endl;


    // cout << jsBuf["id"] << endl;
    // auto arr = jsBuf["id"];
    // cout << arr[2] << endl;
    // auto msgjs = jsBuf["msg"];
    // cout << msgjs << endl;
    // cout << msgjs["name"] <<endl;
    // cout << msgjs["country"] <<endl;

    vector<int> vec = jsBuf["list"];
    for(auto &v: vec)
        cout << v <<" ";
    cout << endl;
    map<int,string> mp = jsBuf["path"];
    for(auto& m:mp)
        cout << m.first << " " << m.second << endl;

    return 0;
}