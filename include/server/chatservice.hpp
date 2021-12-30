#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include <mutex>
#include <memory>
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "redis.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "json.hpp"
using json = nlohmann::json;

//表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn,json &js,Timestamp time)>;


//聊天服务器业务类
class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService* instance();
    //处理登录业务
    void login(const TcpConnectionPtr &con,json &js,Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr &con,json &js,Timestamp time);
    //一对一聊天业务
    void oneChat(const TcpConnectionPtr &con,json &js,Timestamp time);
    //添加好友业务
    void addFriend(const TcpConnectionPtr &con,json &js,Timestamp time);
    //创建群组业务
    void creatGroup(const TcpConnectionPtr &con,json &js,Timestamp time);
    //加入群组业务
    void addGroup(const TcpConnectionPtr &con,json &js,Timestamp time);
    //群组聊天业务
    void groupChat(const TcpConnectionPtr &con,json &js,Timestamp time);
    //注销业务
    void loginout(const TcpConnectionPtr &con,json &js,Timestamp time);

    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
    //服务器异常，业务重置方法
    void reset();

    //从redis消息队列中获取订阅信息
    void handleRedisSubscribeMessage(int userid,string msg);
private:
    ChatService();

    //存储消息id和其对应的业务处理方法
    unordered_map<int,MsgHandler> _msgHandlerMap;

    //存储在线用户的通信连接
    /*
        必须考虑线程安全，因为随着用户上下线，在动态变化...
        msgHandlerMap提前添加好了，所以不用考虑线程安全
    */
    unordered_map<int,TcpConnectionPtr> _userConnMap;

    //添加互斥锁，保证_userConnMap的线程安全
    //C++11已经为多线程编程提供了互斥锁，条件变量
    //而且解锁动作是智能的，自动解锁，防止使用者忘了
    mutex _connMutex;

    //数据类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    //  redis操作对象
    Redis _redis;
};


#endif