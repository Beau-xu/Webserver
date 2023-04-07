#include "Acceptor.h"

#include "Channel.h"
#include "InetAddress.h"
#include "Socket.h"

Acceptor::Acceptor(EventLoop *_loop, std::function<void(Socket *)> _cb)
    : loop(_loop), sock(nullptr), acceptChannel(nullptr), newConnectionCallback(_cb) {
    sock = new Socket();
    InetAddress *addr = new InetAddress("127.0.0.1", 1234);
    sock->bind(addr);
    sock->listen();
    sock->setnonblocking();
    acceptChannel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setCallback(cb);
    acceptChannel->enableReading();
    delete addr;
}

Acceptor::~Acceptor() {
    delete sock;
    delete acceptChannel;
}

void Acceptor::acceptConnection() {
    InetAddress *clnt_addr = new InetAddress();
    Socket *clnt_sock = new Socket(sock->accept(clnt_addr));
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->getFd(),
           inet_ntoa(clnt_addr->getAddr().sin_addr), ntohs(clnt_addr->getAddr().sin_port));
    clnt_sock->setnonblocking();
    newConnectionCallback(clnt_sock);
    delete clnt_addr;
}
