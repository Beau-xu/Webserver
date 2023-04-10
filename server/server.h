#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>

#include <unordered_map>

#include "../epoller/epoller.h"
#include "../heaptimer/heaptimer.h"
#include "../http/httpconn.h"
#include "../log/log.h"
#include "../sqlconn/sqlconn.h"
#include "../sqlconn/sqlconnpool.h"
#include "../threadpool/threadpool.h"

class Server {
   public:
    Server(int port, int trigMode, int timeoutMS, bool OptLinger, int sqlPort, const char* sqlUser,
           const char* sqlPwd, const char* dbName, int connPoolNum, int threadNum, bool openLog,
           int logLevel, int logQueSize);
    ~Server();
    void start();

   private:
    static const int MAX_FD = 65536;
    static int setFdNonblock(int fd);

    bool initSocket_();
    void initEventMode_(int trigMode);
    void addClient_(int fd, sockaddr_in addr);

    void dealListen_();
    void dealWrite_(HttpConn* client);
    void dealRead_(HttpConn* client);

    void sendError_(int fd, const char* info);
    void extentTime_(HttpConn* client);
    void closeConn_(HttpConn* client);

    void onRead_(HttpConn* client);
    void onWrite_(HttpConn* client);
    void onProcess_(HttpConn* client);

    int port_;
    bool openLinger_;
    int timeoutMS_; /* 毫秒MS */
    bool isClose_;
    int listenFd_;
    char* srcDir_;

    uint32_t listenEvent_;
    uint32_t connEvent_;

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};

#endif  // SERVER_H
