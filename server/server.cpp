#include "server.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>  // close()

#include <cassert>

Server::Server(int port, int trigMode, int timeoutMS, bool OptLinger, int sqlPort,
               const char* sqlUser, const char* sqlPwd, const char* dbName, int sqlConnPoolNum,
               int threadNum, bool useLog, int logLevel, int logQueSize)
    : port_(port),
      openLinger_(OptLinger),
      timeoutMS_(timeoutMS),
      isClose_(false),
      timer_(new HeapTimer()),
      threadpool_(new ThreadPool(threadNum)),
      epoller_(new Epoller()) {
    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strncat(srcDir_, "/resources/", 16);
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir_;
    SqlConnPool::instance()->init("localhost", sqlPort, sqlUser, sqlPwd, dbName, sqlConnPoolNum);

    initEventMode_(trigMode);
    if (!initSocket_()) isClose_ = true;
    if (useLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if (isClose_) {
            LOG_ERROR("========== Server initialization failed!==========");
        } else {
            LOG_INFO("========== Server initializing ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, OptLinger ? "true" : "false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s", (listenEvent_ & EPOLLET ? "ET" : "LT"),
                     (connEvent_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", sqlConnPoolNum, threadNum);
        }
    }
}

Server::~Server() {
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SqlConnPool::instance()->closePool();
}

void Server::initEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode) {
        case 0: break;
        case 1: connEvent_ |= EPOLLET; break;
        case 2: listenEvent_ |= EPOLLET; break;
        case 3:
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
        default:
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
    }
    HttpConn::isET = (connEvent_ & EPOLLET);
}

void Server::start() {
    int timeMS = -1;  // epoll wait timeout == -1 无事件将阻塞
    if (!isClose_) LOG_INFO("========== Server start ==========");
    while (!isClose_) {
        // 清除超时节点（关闭客户端连接），返回值是下一个超时连接的剩余时间
        if (timeoutMS_ > 0) timeMS = timer_->getNextTick();
        // epoll监听
        int eventCnt = epoller_->wait(timeMS);
        for (int i = 0; i < eventCnt; i++) {
            int fd = epoller_->getEventFd(i);
            uint32_t events = epoller_->getEvents(i);
            if (fd == listenFd_) {
                dealListen_();
            } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                closeConn_(&users_[fd]);
            } else if (events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                dealRead_(&users_[fd]);
            } else if (events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                dealWrite_(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void Server::sendError_(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) LOG_WARN("failed to send error to client[%d]!", fd);
    close(fd);
}

void Server::closeConn_(HttpConn* client) {
    assert(client);
    LOG_INFO("close Client[%d]!", client->getFd());
    epoller_->delFd(client->getFd());
    client->closeHttp();
}

void Server::addClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if (timeoutMS_ > 0) {  // 给客户端连接添加计时器
        timer_->add(fd, timeoutMS_, std::bind(&Server::closeConn_, this, &users_[fd]));
    }
    epoller_->addFd(fd, EPOLLIN | connEvent_);
    setFdNonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].getFd());
}

void Server::dealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (sockaddr*)&addr, &len);
        if (fd <= 0) return;
        if (HttpConn::userCount >= MAX_FD) {
            sendError_(fd, "Server busy!");
            close(fd);
            LOG_WARN("Clients is full!");
            return;
        }
        addClient_(fd, addr);
    } while (listenEvent_ & EPOLLET);
}

void Server::dealRead_(HttpConn* client) {
    assert(client);
    extentTime_(client);
    threadpool_->addTask(std::bind(&Server::onRead_, this, client));
}

void Server::dealWrite_(HttpConn* client) {
    assert(client);
    extentTime_(client);
    threadpool_->addTask(std::bind(&Server::onWrite_, this, client));
}

void Server::extentTime_(HttpConn* client) {
    assert(client);
    if (timeoutMS_ > 0) timer_->adjust(client->getFd(), timeoutMS_);
}

void Server::onRead_(HttpConn* client) {
    assert(client);
    int readErrno = 0;
    if (client->readFd(&readErrno) <= 0 && readErrno != EAGAIN && readErrno != EWOULDBLOCK) {
        // httpconn read buffer读取数据
        closeConn_(client);
        return;
    }
    onProcess_(client);
}

void Server::onProcess_(HttpConn* client) {
    if (client->process()) {  // 根据 httpconn 读缓存解析报文，生成response并写入写缓存
        epoller_->modFd(client->getFd(), connEvent_ | EPOLLOUT);
    } else {
        epoller_->modFd(client->getFd(), connEvent_ | EPOLLIN);
    }
}

void Server::onWrite_(HttpConn* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->writeFd(&writeErrno);
    if (client->toWriteBytes() == 0) {  // 传输完成
        if (client->isKeepAlive()) {
            onProcess_(client);
            return;
        }
    } else if (ret < 0 && writeErrno == EAGAIN) {  // 继续传输
        epoller_->modFd(client->getFd(), connEvent_ | EPOLLOUT);
        return;
    }
    closeConn_(client);
}

/* Create listenFd */
bool Server::initSocket_() {
    struct sockaddr_in addr;
    if (port_ > 65535 || port_ < 1024) {
        LOG_ERROR("Port:%d error!", port_);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    struct linger optLinger = {0};
    if (openLinger_) {  // 直到所剩数据发送完毕或超时才关闭
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        LOG_ERROR("Create socket error!", port_);
        return false;
    }
    if (setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger)) == -1) {
        close(listenFd_);
        LOG_ERROR("Init linger error!", port_);
        return false;
    }

    int optval = 1;
    if (setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }
    if (bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    if (listen(listenFd_, 6) < 0) {
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    if (!epoller_->addFd(listenFd_, listenEvent_ | EPOLLIN)) {
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }
    setFdNonblock(listenFd_);
    LOG_INFO("Server port:%d", port_);
    return true;
}

int Server::setFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
