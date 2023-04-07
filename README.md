# WebServer
My Web Server in C++11

## Socket
- fd文件描述符，即一个事件。创建时可指定fd
- 绑定：将（服务器）fd和InetAddress绑定
- 接受：当客户端连接到服务器socket时，通过accept函数接收客户端InetAddr并返回客户端socket。

## Epoll
- epfd文件描述符df
- struct epoll_event *events 被监听的事件（封装了sockfd的Channel）序列
- updateChannel，把channel及其指定的事件类型添加到事件序列中
- epoll监听到服务器的fd事件，则服务器Socket运行accept，取得客户端sockfd并新建Socket对象，并开始监听客户端。

## Channel
- 管理Epoll、sockfd、事件类型（要执行的操作）
- sockfd加入该epoll的事件序列
- events指定事件类型。如enablereading()即设置events为读`EPOLLIN`，然后执行`Epoll.updateChannel()`添加/更新事件
-
