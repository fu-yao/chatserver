#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

//记录当前系统登录的用户信息
User g_currentUser;
//记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
//记录当前登录用户的群列表信息
vector<Group> g_currentUserGroupList;
//显示当前登录成功用户的基本信息
void showCurrentUserData();
//控制主菜单页面程序
bool isMainMenuRunning = false;

//接受线程
void readTaskHandler(int clientfd);
//获取系统事件(聊天信息需要添加事件变量)    c++11的方法
string getCurrentTime();
//主聊天页面程序
void mainMenu(int clientfd);

//聊天客户端程序实现，main线程用作发送线程，子线程用作接受线程
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << endl;
        exit(-1);
    }

    //解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建client端的socket  SOCK_STREAM => TCP
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

    //填写client需要连接的server信息ip+port
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    //client 和 server 进行连接
    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    //main线程用于接受用户输入，负责发送数据
    for (;;)
    {
        //显示页面菜单、登录、注册、退出
        cout << "========================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice; //读取一个整数，还残留回车
        cin.get();     //读掉缓冲区残留的回车

        switch (choice)
        {
        case 1: // login业务
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid:";
            cin >> id;
            cin.get(); //读掉缓冲区残留的回车
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            cout << "登录MSG: " << request << endl;

            if (len == -1)
            {
                cerr << "send login msg error:" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};

                len = recv(clientfd, buffer, 1024, 0);
                cout << "recv: " << len << endl;
                if (-1 == len)
                {
                    cerr << "recv login reponse error" << endl;
                }
                else    //登录成功
                {
                    json responsejs = json::parse(buffer);
                    if (0 != responsejs["errno"].get<int>()) //登录失败
                    {
                        cerr << responsejs["errmsg"] << endl;
                    }
                    else //登录成功
                    {
                        //  记录当前用户的id和name
                        cout << "进没进来2" << endl;
                        g_currentUser.setId(responsejs["id"].get<int>());
                        g_currentUser.setName(responsejs["name"]);

                        //  记录当前用户的好友列表消息
                        //  判断js里面是否包含“friends”的键
                        if (responsejs.contains("friends"))
                        {
                            //初始化
                            g_currentUserFriendList.clear();

                            vector<string> vec = responsejs["friends"];
                            for (string &str : vec)
                            {
                                json js = json::parse(str);
                                User user;
                                user.setId(js["id"].get<int>());
                                user.setName(js["name"]);
                                user.setState(js["state"]);
                                g_currentUserFriendList.push_back(user);
                            }
                        }

                        //  记录当前用户的群组列表消息
                        //  判断js里面是否包含“groups”的键
                        if (responsejs.contains("groups"))
                        {
                            //初始化
                            g_currentUserGroupList.clear();

                            vector<string> vec1 = responsejs["groups"];
                            for (string &groupstr : vec1)
                            {
                                json grpjs = json::parse(groupstr);
                                Group group;
                                group.setId(grpjs["id"].get<int>());
                                group.setName(grpjs["groupname"]);
                                group.setDesc(grpjs["groupdesc"]);

                                vector<string> vec2 = grpjs["users"];
                                for (string &userstr : vec2)
                                {
                                    GroupUser user;
                                    json js = json::parse(userstr);
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    user.setRole(js["role"]);
                                    group.getUsers().push_back(user);
                                }

                                g_currentUserGroupList.push_back(group);
                            }
                        }

                        //显示登录用户的基本信息
                        showCurrentUserData();
                        cout << "冷路陈工" << endl;

                        //显示当前用户的离线消息    个人聊天消息或者群组消息
                        if (responsejs.contains("offlinemsg"))
                        {
                            vector<string> vec = responsejs["offlinemsg"];
                            for (string &str : vec)
                            {
                                json js = json::parse(str);
                                //  time + [id] + name + " said: " + xxx
                                if (ONE_CHAT_MSG == js["msgid"].get<int>())
                                {
                                    cout << "个人消息[" << js["id"] << "] " << js["time"].get<string>() << " ["
                                         << js["name"].get<string>() << "] "
                                         << " said: " << js["msg"].get<string>() << endl;
                                }
                                else
                                {
                                    cout << "群聊消息[" << js["groupid"] << "] " << js["time"].get<string>() << " ["
                                         << js["id"] << "] "
                                         << " said: " << js["msg"].get<string>() << endl;
                                }
                            }
                        }

                        //登录成功，启动接受线程负责接受数据；
                        static int readthreadnumber = 0;
                        if(readthreadnumber == 0)
                        {
                            std::thread readTask(readTaskHandler, clientfd); //相当于调用一个pthread_create
                            readTask.detach();      //相当于调用pthread_detach  设置一个分离线程，线程用完自动回收线程所调用的内核的pcd资源
                            readthreadnumber++;
                        }
                        // std::thread readTask(readTaskHandler, clientfd); //相当于调用一个pthread_create
                        // readTask.detach();                               //相当于调用pthread_detach  设置一个分离线程，线程用完自动回收线程所调用的内核的pcd资源

                        //进入聊天主菜单页面
                        isMainMenuRunning = true;
                        mainMenu(clientfd);
                    }
                }
            }
        }
        break;
        case 2: //register业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:";
            cin.getline(name, 50); // cin >> zhang san 只读到zhang    c语言中用gets
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send reg msg error:" << request << endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if (-1 == len)
                {
                    cerr << "recv reg response error" << endl;
                }
                else
                {
                    json responsejs = json::parse(buffer); //反序列化
                    if (0 != responsejs["errno"].get<int>())
                    {
                        cerr << name << "is already exist, register error" << endl;
                    }
                    else
                    {
                        cout << name << " register success, userid is" << responsejs["id"]
                             << ", do not forget it!" << endl;
                    }
                }
            }
        }
        break;
        case 3: //quit业务
            close(clientfd);
            exit(0);
        default:
            cerr << "invalid input!" << endl;
            break;
        }
    }

    return 0;
}

//接受线程
void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (-1 == len || 0 == len)
        {
            close(clientfd);
            exit(-1);
        }

        //接受ChatServer发送过来的数据,反序列化生成json数据对象
        json js = json::parse(buffer);
        cout << "接受Server，反序列化后:"  << js << endl;
        int msgtype = js["msgid"].get<int>();
        //一对一消息
        if (ONE_CHAT_MSG == msgtype)
        {
            cout << "个人消息[" << js["id"] << "] " << js["time"].get<string>() << " ["
                 << js["name"].get<string>() << "] "
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        if (GROUP_CHAT_MSG == msgtype)
        {
            cout << "群聊消息[" << js["groupid"] << "] " << js["time"].get<string>() << " ["
                 << js["id"] << "] "
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        
        //群组消息
    }
}

void showCurrentUserData()
{
    cout << "=======================login user=======================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "-----------------------friend list ---------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "-----------------------group list-----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState()
                     << user.getRole() << endl;
            }
        }
    }
    cout << "========================================================" << endl;
}

// "help" command handler
void help(int fd = 0, string str = "");
// "chat" command handler
void chat(int, string);
//  "addfriend" command handler
void addfriend(int, string);
//  "creategroup" command handler
void creategroup(int, string);
//  "addgroup"  command handler
void addgroup(int, string);
//  "groupchat" command handler
void groupchat(int, string);
//  "quit" command handler
void loginout(int, string);

//  系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creategroup", "创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组,格式groupchat:groupid:message"},
    {"groupchat", "群聊,格式groupchat:groupid:message"},
    {"loginout", "注销,格式loginout"}};

//  注册系统支持的客户端命令处理
//  因为是基于命令行的操作，用户输入可能不合法，因此限定输入，非法就报错
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

//  主聊天页面程序
void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; //存储命令
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }

        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        //找到命令
        //调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }
}

//  "help" command handler
void help(int, string)
{
    cout << "show command list >>> " << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

//  "addfriend" command handler
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfirend msg error -> " << buffer << endl;
    }
}

// "chat" command handler
void chat(int clientfd, string str)
{
    int idx = str.find(":"); //friendid:message
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send chat msg error -> " << buffer << endl;
    }
}

//  "creategroup" command handler
void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    //第一个参数是下标，第二个是长度
    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send creategroup msg error -> " << buffer << endl;
    }
}

//  "addgroup"  command handler
void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send creategroup msg error -> " << buffer << endl;
    }
}

//  "groupchat" command handler     groupid:message
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }

    json js;
    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()), 0);
}

//  "quit" command handler
void loginout(int clientfd, string str)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "sen loginout msg error -> " << buffer << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
}

//获取系统事件(聊天信息需要添加事件信息)
string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}
