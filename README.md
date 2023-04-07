# WebServer
My Web Server in C++11

## Socket
- fd文件描述符，即一个事件。创建时可指定fd
- 绑定：将（服务器）fd和InetAddress绑定
- 接受：当客户端连接到服务器socket时（由epoll监听），通过accept函数接收客户端InetAddr并返回客户端socket。

## Epoll
- epfd文件描述符df
- `struct epoll_event *events`被监听的事件（封装了sockfd的Channel）序列
- `updateChannel`把channel及其指定的事件类型添加到事件序列中
- epoll监听到服务器的fd事件，则服务器Socket运行accept，取得客户端sockfd、新建Socket对象，并开始监听客户端。

## Eventloop
- 初始化时创建Epoll对象
- `loop`循环过程添加

## Channel
- 绑定*Eventloop、sockfd，及其对应事件类型（要执行的操作）
- sockfd加入该epoll的事件序列
- events指定事件类型。由Channel设置并执行`Eventloop.Epoll.updateChannel(*Channel)`添加/更新事件
- `std::function<void()> callback`指定要执行的函数，由Server赋值。

## Server
- 初始化时创建Socket，绑定InetAddr；创建Channel，设置callback
- callback设置：服务器Channel为客户端创建新Channel并设置客户端的callback
- 服务器根据客户端Channel的event类型设置callback，如读、写等任务
