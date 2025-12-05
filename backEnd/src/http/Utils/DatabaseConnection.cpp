#include "DatabaseConnection.h"
#include "config/config.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <thread>

DatabaseConnection::DatabaseConnection() : conn(nullptr)
{
    init();
}

DatabaseConnection::~DatabaseConnection()
{
    closeConnection();
}

bool DatabaseConnection::init()
{
    try
    {
        // 从配置中获取数据库连接参数
        Config& config = Config::getInstance();
        host = config.getDatabaseHost();
        user = config.getDatabaseUsername();
        password = config.getDatabasePassword();
        database = config.getDatabaseName();
        port = config.getDatabasePort();

        // 连接重试次数
        int maxRetries = 3;
        int retryCount = 0;
        bool connected = false;

        while (retryCount < maxRetries && !connected)
        {
            // 初始化数据库连接
            MYSQL* temp_conn = mysql_init(nullptr);
            if (!temp_conn)
            {
                spdlog::error("无法初始化MySQL连接");
                retryCount++;
                continue;
            }

            // 设置连接超时和重试参数
            mysql_options(temp_conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
            mysql_options(temp_conn, MYSQL_OPT_READ_TIMEOUT, "300"); // 增加读取超时到5分钟
            mysql_options(temp_conn, MYSQL_OPT_WRITE_TIMEOUT, "300"); // 增加写入超时到5分钟
            mysql_options(temp_conn, MYSQL_INIT_COMMAND, "SET NAMES utf8mb4");
            mysql_options(temp_conn, MYSQL_INIT_COMMAND, "SET CHARACTER SET utf8mb4");
            mysql_options(temp_conn, MYSQL_INIT_COMMAND, "SET SQL_MODE='NO_ENGINE_SUBSTITUTION'");
            // 移除弃用的MYSQL_OPT_RECONNECT选项，使用手动重连机制

            // 建立连接
            if (!mysql_real_connect(temp_conn, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0))
            {
                spdlog::error("连接数据库失败 (重试 {}): {}", retryCount + 1, mysql_error(temp_conn));
                mysql_close(temp_conn);
                retryCount++;
                // 等待一段时间后重试
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
            else
            {
                // 连接成功，赋值给成员变量
                conn = temp_conn;
                spdlog::info("数据库连接成功");
                connected = true;
            }
        }

        return connected;
    }
    catch (const std::exception& e)
    {
        spdlog::error("初始化数据库连接失败: {}", e.what());
        if (conn)
        {
            mysql_close(conn);
            conn = nullptr;
        }
        return false;
    }
}

MYSQL* DatabaseConnection::getConnection()
{
    // 线程锁保护，确保连接的安全访问和修改
    std::lock_guard<std::mutex> lock(connectionMutex);
    
    // 检查连接是否有效
    bool isConnected = false;
    
    if (conn != nullptr)
    {
        // 使用mysql_ping来检查连接是否有效，这是一个安全的方法
        // mysql_ping会自动处理未处理的结果集，并尝试重新连接（如果需要）
        if (mysql_ping(conn) == 0)
        {
            isConnected = true;
        }
        else
        {
            spdlog::error("数据库连接已失效，错误信息: {}", mysql_error(conn));
            // 关闭当前无效连接
            mysql_close(conn);
            conn = nullptr;
        }
    }
    
    // 如果连接无效，尝试重新连接
    if (!isConnected)
    {
        spdlog::info("数据库连接无效，尝试重新连接");
        if (!reconnect())
        {
            spdlog::error("重新连接数据库失败");
            return nullptr;
        }
    }
    return conn;
}

void DatabaseConnection::closeConnection()
{
    // 线程锁保护，确保连接的安全关闭
    std::lock_guard<std::mutex> lock(connectionMutex);
    
    if (conn)
    {
        mysql_close(conn);
        conn = nullptr;
        spdlog::info("数据库连接已关闭");
    }
}

bool DatabaseConnection::reconnect()
{
    // 线程锁保护，确保重新连接的安全执行
    std::lock_guard<std::mutex> lock(connectionMutex);
    
    // 直接关闭现有连接，避免重复获取锁
    if (conn)
    {
        mysql_close(conn);
        conn = nullptr;
        spdlog::info("数据库连接已关闭");
    }
    
    // 重新初始化连接
    return init();
}
