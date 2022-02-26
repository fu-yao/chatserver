### 项目介绍

本项目主要是在Linux环境下，基于muduo网络库，使用C++开发的一个简易的聊天服务器。目前实现的业务功能有登录、注册、单聊、群聊等。本次项目重点不在于丰富的业务功能，而是使用muduo网络库进行服务端程序设计以及实战上学会了使用nginx的负载均衡功能、redis的发布订阅功能，以mysql进行数据落地存储，用json序列化和反序列化作为通信协议。

### 项目环境：

1.g++7.3以上

2.CMake-Version 3.0(项目构建管理)

3.Nginx(基于TCP的负载均衡，编译时需要加上--with-stream)

4.Redis(主要使用的是一个发布-订阅功能)

5.Mysql(数据落地存储)

6.Muduo网络库最新版，详情可去陈硕大神github上了解(核心网络模块)

7.json使用的是JSON for Modern C++(序列化和反序列化)

另附：

muduo地址：https://github.com/chenshuo/muduo

json地址：https://github.com/nlohmann/json

### 编译

```
chmod +x autobuild.sh
./autobuild.sh
```

### 运行

启动多个服务器，再启动多个客户端，根据ngnix的轮询算法，客户端连接会被依次分发到不同服务器，实现负载均衡。

```
cd bin
./ChatServer 127.0.0.1 6000	//注意这里，需要在ngnix上提前设置好要监听的server ip和port,才能使用负载均衡功能

./ChatClient 127.0.0.1 8000	//注意此时客户端端口为ngnix上conf文件中stream模块设置的listen端口
```

### 使用样例
![图片](https://user-images.githubusercontent.com/84277885/155265144-221d2713-4a04-4671-a982-398dff79b289.png)


### 性能测试

```
压力测试:测试集群环境下的并发量。
压测环境:本机开启虚拟机2核4g做测试ChatRoomClient
	2核4g的腾讯云部署Nginx服务器实现基于tcp长连接的负载均衡
	2核4g的腾讯云部署ChatRoomServer，部署了三台
	2核2g的腾讯云数据库部署mysql，
压测结果：测试并发场景下的最大连接数:单机大概17000左右，三台服务器并发量去到5w左右。
```

连接池性能测试(仅使用插入操作做了简单测试)
| 数据量 | 未使用连接池花费时间(ms) | 使用连接池花费时间          |
| ------ | ------------------------ | --------------------------- |
| 1000   | 单线程:   四线程:7954    | 单线程: 4252    四线程:1745 |
| 5000   | 单线程:   四线程:39067   | 单线程:21126   四线程:8614  |
| 10000  | 单线程: 四线程:78095     | 单线程:41283  四线程:16773  |