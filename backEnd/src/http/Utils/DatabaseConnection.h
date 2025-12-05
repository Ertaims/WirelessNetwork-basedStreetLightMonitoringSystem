#pragma once

#include <mysql/mysql.h>
#include <string>
#include <memory>
#include <mutex>

class DatabaseConnection
{
public:
    // 构造函数和析构函数
    DatabaseConnection();
    ~DatabaseConnection();

    // 获取数据库连接
    MYSQL* getConnection();

    // 关闭数据库连接
    void closeConnection();

    // 重新连接数据库
    bool reconnect();

private:
    // 初始化数据库连接
    bool init();

    // 数据库连接句柄
    MYSQL* conn;
    
    // 连接参数
    std::string host;
    std::string user;
    std::string password;
    std::string database;
    int port;
    
    // 线程锁，用于保护连接的访问和修改
    std::mutex connectionMutex;
};
