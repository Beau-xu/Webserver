# WebServer
My Web Server in C++11

## Socket
- fd文件描述符，即一个事件。创建时可指定fd
- 绑定：将（服务器）fd和InetAddress绑定
- 接受：当客户端连接到服务器socket时（由epoll监听），通过accept函数接收客户端InetAddr并返回客户端socket。

## Epoll
- epfd文件描述符df
- `struct epoll_event *events`被监听的事件（封装了sockfd的Channel）序列
- `updateChannel`把channel及其指定的事件类型添加到epoll监听事件中
- epoll监听到服务器的fd事件，则服务器Socket运行accept，取得客户端sockfd、新建Socket对象，并开始监听客户端。

## Eventloop
- *Epool，初始化时创建Epoll对象
- `loop`每次循环取一系列封装了就绪的Socket的Channel，并依次执行各Channel的callback函数，完成任务。

## Channel
封装Socket及其事件类型，便于根据类型分配不同任务（callback）；可通过Channel注册事件到Eventloop的epoll中
- 绑定*Eventloop、sockfd，及其对应事件类型（要执行的操作）
- sockfd加入该epoll的事件序列
- events指定事件类型。由Channel设置并执行`Eventloop.Epoll.updateChannel(*Channel)`添加/更新epoll监听的事件
- `callback`指定要执行的函数，是Server成员函数中的一个。

## Acceptor
存在于事件驱动EventLoop类中，即Reactor模式的main-Reactor，保存服务器的socket fd。
- 初始化时，创建服务器的Socket、Channel，由epoll监听，并设置服务器的callback：为客户端分发，由Server初始化时设置。
- 负责创建Connection

## Connection
存在于事件驱动EventLoop类中，即Reactor模式的main-Reactor，保存客户端的socket fd。
- 初始化时创建Socket、Channel、设置（客户端的）callback，callback为Connection的成员函数。
- 通过Channel分发到epoll，该Channel的事件处理函数handleEvent()会调用Connection中的事件处理函数来响应客户端请求

## Server
- 初始化时创建Acceptor，设置其callback：`Server::newConnection`成员函数，即Acceptor的任务
- 持有Acceptor指针
- 持有全部客户端的连接（Connection），以`map<int, Connection*>`形式保留fd到Connection对象的映射
