#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>

#include <condition_variable>
#include <mutex>
#include <queue>

#include "../log/log.h"

class SqlConnPool {
   public:
    static SqlConnPool* instance();
    void init(const char* host, int port, const char* user, const char* pwd, const char* dbName,
              int connSize);
    MYSQL* getConn();
    void freeConn(MYSQL* conn);
    int getFreeCount();
    void closePool();

   private:
    SqlConnPool();
    ~SqlConnPool();

    std::queue<MYSQL*> queConn_;
    std::mutex mtx_;
    std::condition_variable condVar_;
};

#endif  // SQLCONNPOOL_H
