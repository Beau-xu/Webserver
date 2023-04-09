#ifndef SQLCONN_H
#define SQLCONN_H

#include "sqlconnpool.h"

class SqlConnection {
   public:
    SqlConnection(MYSQL** sql, SqlConnPool* connpool) {
        assert(connpool);
        *sql = connpool->getConn();
        sql_ = *sql;
        connpool_ = connpool;
    }

    ~SqlConnection() {
        if (sql_) {
            connpool_->freeConn(sql_);
        }
    }

   private:
    MYSQL* sql_;
    SqlConnPool* connpool_;
};

#endif  // SQLCONN_H
