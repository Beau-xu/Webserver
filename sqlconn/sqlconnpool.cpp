#include "sqlconnpool.h"

#include <thread>

SqlConnPool::SqlConnPool() {}

SqlConnPool* SqlConnPool::instance() {
    static SqlConnPool connPool;
    return &connPool;
}

void SqlConnPool::init(const char* host, int port, const char* user, const char* pwd,
                       const char* dbName, int connSize = 10) {
    assert(connSize > 0);
    for (int i = 0; i < connSize; i++) {
        MYSQL* sql = nullptr;
        sql = mysql_init(sql);
        if (!sql) {
            LOG_ERROR("MySql init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        if (!sql) {
            LOG_ERROR("MySql Connect error!");
        }
        queConn_.push(sql);
    }
    // max_conn_ = connSize;
}

MYSQL* SqlConnPool::getConn() {
    MYSQL* sql = nullptr;
    std::unique_lock<std::mutex> locker(mtx_);
    if (queConn_.empty()) condVar_.wait(locker);
    sql = queConn_.front();
    queConn_.pop();
    return sql;
}

void SqlConnPool::freeConn(MYSQL* sql) {
    assert(sql);
    std::lock_guard<std::mutex> locker(mtx_);
    queConn_.push(sql);
    condVar_.notify_one();
}

void SqlConnPool::closePool() {
    std::lock_guard<std::mutex> locker(mtx_);
    while (!queConn_.empty()) {
        auto item = queConn_.front();
        queConn_.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

int SqlConnPool::getFreeCount() {
    std::lock_guard<std::mutex> locker(mtx_);
    return queConn_.size();
}

SqlConnPool::~SqlConnPool() { closePool(); }
