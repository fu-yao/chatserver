#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
using namespace std;

// 处理服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);

    EventLoop loop; //epoll
    InetAddress addr(port,ip);
    ChatServer server(&loop, addr, "ChatServer");
    //cout << "SOCKMAXCONN:" << SOMAXCONN << endl;

    server.start();     //listenfd epoll_ctl => epoll
    loop.loop();        //epoll_wait以阻塞方式等待新用户连接,已连接用户的读写时间等

    return 0;
}