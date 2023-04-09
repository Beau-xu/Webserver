#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <arpa/inet.h>  // sockaddr_in
#include <stdlib.h>  // atoi()
#include <atomic>
#include <sys/types.h>
#include <sys/uio.h>  // readv/writev

#include "../log/log.h"
#include "../sqlconn/sqlconn.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn {
   public:
    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;

    HttpConn();
    ~HttpConn();

    void init(int sockFd, const sockaddr_in& addr);
    ssize_t readFd(int* saveErrno);
    ssize_t writeFd(int* saveErrno);
    void closeHttp();
    int getFd() const;
    int getPort() const;
    const char* getIP() const;
    sockaddr_in getAddr() const;
    bool process();

    int toWriteBytes() { return iov_[0].iov_len + iov_[1].iov_len; }

    bool isKeepAlive() const { return request_.isKeepAlive(); }

   private:
    int fd_;
    struct sockaddr_in addr_;
    bool isClose_;
    int iovCnt_;
    struct iovec iov_[2];

    std::string readBuff_;   // 读缓冲区
    std::string writeBuff_;  // 写缓冲区
    HttpRequest request_;
    HttpResponse response_;
};

#endif  // HTTPCONN_H
