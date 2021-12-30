/*

muduo网络库主要提供了两个类
TcpServer : 用于编写服务器程序
TcpClient : 用于编写客户端程序

如果自己编写
epoll + 线程池
用muduo的(libevent)，就封装好了。
好处：能够把网络I/O的代码和业务代码区分开
                    业务代码(主要)：用户的连接和断开        用户的可读写事件
*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <string>
#include <functional>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

/*基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建EventLoo事件循环对象的指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
4.在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写事件的回调函数
5.设置何使的服务端线程数量，muduo库会自己分配I/O线程和worker线程
*/

class ChatServer
{
public:
    ChatServer(EventLoop *loop,
                const InetAddress &listenAddr,
                const string &nameArg)
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        //给服务器注册用户连接的创建和断开回调                                  占位符
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        //给服务器注册用户读写
        _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));

            //设计服务器端的线程数量    1个I/O线程    3个worker线程
        _server.setThreadNum(1);
    };

    //开启事件循环
    void start()
    {
        _server.start();
    }

private:
    //专门处理用户的连接创建和断开  epoll  litsenfd  accept
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected()){
            cout << conn->peerAddress().toIpPort() << " -> "
                 << conn->localAddress().toIpPort() << " state:online" << endl;
        }else{
            cout << conn->peerAddress().toIpPort() << " -> "
                 << conn->localAddress().toIpPort() << " state:offline" << endl;
            conn->shutdown();   //close(fd)
            //_loop->quit();    
        }
    }

    //  专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn, //连接
                   Buffer *buffer,                  //缓冲区，提高读写性能
                   Timestamp time)               //接受到数据的时间信息
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data:" << buf << " time:" << time.toString() << endl;
        conn->send(buf);
    }

    TcpServer _server; // #1
    EventLoop * _loop;  // #2 epoll
};

int main()
{
    EventLoop loop;
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"ChatServer");

    server.start(); //listenfd epoll_ctl=>epoll
    loop.loop();    //相当于epoll_wait 以阻塞方式等待新用户连接，已连接用户的读写事件等

    return 0;
}