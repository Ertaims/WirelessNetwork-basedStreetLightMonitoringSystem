#include "UserRepository.h"
#include "Utils/PasswordHasher.h"
#include "config/config.h"
#include <mysql/mysql.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

UserRepository::UserRepository() : db(nullptr)
{
    initDatabase();
}

UserRepository::~UserRepository()
{
    if(db)
    {
        mysql_close(static_cast<MYSQL*>(db));
    }
}

void UserRepository::initDatabase()
{
    try
    {
        // 初始化MySQL连接
        mysql_library_init(0, nullptr, nullptr);

        // 初始化数据库连接
        MYSQL* conn = mysql_init(nullptr);
        if(!conn)
        {
            throw std::runtime_error("无法初始化MySQL连接");
        }

        // 从配置中获取数据库连接参数
        Config& config = Config::getInstance();
        const char* host = config.getDatabaseHost().c_str();
        const char* user = config.getDatabaseUsername().c_str();
        const char* password = config.getDatabasePassword().c_str();
        const char* database = config.getDatabaseName().c_str();
        int port = config.getDatabasePort();

        // 打印数据库连接参数（仅用于调试）
        spdlog::info("数据库连接参数: 主机={}, 端口={}, 用户名={}, 数据库名={}", host, port, user, database);

        // 建立连接
        if (!mysql_real_connect(conn, host, user, password, database, port, nullptr, 0)) {
            spdlog::error("连接数据库失败: {}", mysql_error(conn));
            spdlog::error("尝试连接的参数: 主机={}, 端口={}, 用户名={}, 数据库名={}", 
                         host, port, user, database);
            mysql_close(conn);
            throw std::runtime_error("连接数据库失败");
        }
        
        // 将连接保存到成员变量
        db = conn;
    }
    catch(const std::exception& e)
    {
        spdlog::error("初始化数据库错误: {}", e.what());
        throw;
    }
}

bool UserRepository::createUser(const User &user)
{
    try
    {
        MYSQL* conn = static_cast<MYSQL*>(db);
        if (!conn) {
            // 数据库连接无效，返回失败
            spdlog::error("数据库连接无效，无法创建用户");
            return false;
        }

        // 准备SQL语句
        std::string sql = "INSERT INTO users (username, password, name, role, email, phone, last_login, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, NULL, NOW(), NOW())";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            throw std::runtime_error("无法初始化SQL语句");
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("准备SQL语句失败: " + error);
        }

        // 绑定参数
        MYSQL_BIND bind[6];
        memset(bind, 0, sizeof(bind));

        // username
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)user.getUsername().c_str();
        bind[0].buffer_length = user.getUsername().length();

        // password
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)user.getPasswordHash().c_str();
        bind[1].buffer_length = user.getPasswordHash().length();

        // name
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)user.getName().c_str();
        bind[2].buffer_length = user.getName().length();

        // role
        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = (char*)user.getRole().c_str();
        bind[3].buffer_length = user.getRole().length();

        // email
        bind[4].buffer_type = MYSQL_TYPE_STRING;
        bind[4].buffer = (char*)user.getEmail().c_str();
        bind[4].buffer_length = user.getEmail().length();

        // phone
        bind[5].buffer_type = MYSQL_TYPE_STRING;
        bind[5].buffer = (char*)user.getPhone().c_str();
        bind[5].buffer_length = user.getPhone().length();

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定参数失败: " + error);
        }

        // 执行语句
        if (mysql_stmt_execute(stmt) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("执行SQL语句失败: " + error);
        }

        // 检查影响的行数
        my_ulonglong affectedRows = mysql_stmt_affected_rows(stmt);
        mysql_stmt_close(stmt);

        return affectedRows > 0;
    }
    catch(const std::exception& e)
    {
        spdlog::error("创建用户失败: {}", e.what());
        return false;
    }
}

std::shared_ptr<User> UserRepository::getUserByUsername(const std::string &username)
{
    try
    {
        MYSQL* conn = static_cast<MYSQL*>(db);
        if (!conn) {
            // 数据库连接无效，返回空指针
            spdlog::error("数据库连接无效，无法通过用户名获取用户");
            return nullptr;
        }

        // 准备SQL语句
        std::string sql = "SELECT id, username, password, role, created_at FROM users WHERE username = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            throw std::runtime_error("无法初始化SQL语句");
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("准备SQL语句失败: " + error);
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)username.c_str();
        bind[0].buffer_length = username.length();

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定参数失败: " + error);
        }

        // 执行查询
        if (mysql_stmt_execute(stmt) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("执行查询失败: " + error);
        }

        // 绑定结果
        MYSQL_BIND resultBind[5];
        memset(resultBind, 0, sizeof(resultBind));

        int id = 0;
        char usernameResult[51];
        char passwordHash[256];
        char role[20];
        char createdAt[21];

        resultBind[0].buffer_type = MYSQL_TYPE_LONG;
        resultBind[0].buffer = &id;

        resultBind[1].buffer_type = MYSQL_TYPE_STRING;
        resultBind[1].buffer = usernameResult;
        resultBind[1].buffer_length = sizeof(usernameResult);
        resultBind[1].length = new unsigned long;

        resultBind[2].buffer_type = MYSQL_TYPE_STRING;
        resultBind[2].buffer = passwordHash;
        resultBind[2].buffer_length = sizeof(passwordHash);
        resultBind[2].length = new unsigned long;

        resultBind[3].buffer_type = MYSQL_TYPE_STRING;
        resultBind[3].buffer = role;
        resultBind[3].buffer_length = sizeof(role);
        resultBind[3].length = new unsigned long;

        resultBind[4].buffer_type = MYSQL_TYPE_STRING;
        resultBind[4].buffer = createdAt;
        resultBind[4].buffer_length = sizeof(createdAt);
        resultBind[4].length = new unsigned long;

        if (mysql_stmt_bind_result(stmt, resultBind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            // 清理内存
            delete resultBind[1].length;
            delete resultBind[2].length;
            delete resultBind[3].length;
            delete resultBind[4].length;
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定结果失败: " + error);
        }

        // 获取结果
        if (mysql_stmt_fetch(stmt) == 0) {
            // 创建用户对象
            auto user = std::make_shared<User>();
            user->setId(id);
            user->setUsername(std::string(usernameResult, *resultBind[1].length));
            user->setPasswordHash(std::string(passwordHash, *resultBind[2].length));
            user->setRole(std::string(role, *resultBind[3].length));
            user->setCreatedAt(std::string(createdAt, *resultBind[4].length));

            // 输出结果
            spdlog::info("获取用户成功: {}", user->getUsername());

            // 清理内存
            delete resultBind[1].length;
            delete resultBind[2].length;
            delete resultBind[3].length;
            delete resultBind[4].length;
            mysql_stmt_close(stmt);

            return user;
        }

        // 清理内存
        delete resultBind[1].length;
        delete resultBind[2].length;
        delete resultBind[3].length;
        delete resultBind[4].length;
        mysql_stmt_close(stmt);

        return nullptr;
    }
    catch(const std::exception& e)
    {
        spdlog::error("通过用户名获取用户失败: {}", e.what());
        return nullptr;
    }
}

std::shared_ptr<User> UserRepository::getUserById(int id)
{
    try
    {
        MYSQL* conn = static_cast<MYSQL*>(db);
        if (!conn) {
            // 数据库连接无效，返回空指针
            spdlog::error("数据库连接无效，无法通过ID获取用户");
            return nullptr;
        }

        // 准备SQL语句
        std::string sql = "SELECT id, username, password, role, created_at FROM users WHERE id = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            throw std::runtime_error("无法初始化SQL语句");
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("准备SQL语句失败: " + error);
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定参数失败: " + error);
        }

        // 执行查询
        if (mysql_stmt_execute(stmt) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("执行查询失败: " + error);
        }

        // 绑定结果
        MYSQL_BIND resultBind[5];
        memset(resultBind, 0, sizeof(resultBind));

        int idResult = 0;
        char username[51];
        char passwordHash[256];
        char role[20];
        char createdAt[21];

        resultBind[0].buffer_type = MYSQL_TYPE_LONG;
        resultBind[0].buffer = &idResult;

        resultBind[1].buffer_type = MYSQL_TYPE_STRING;
        resultBind[1].buffer = username;
        resultBind[1].buffer_length = sizeof(username);
        resultBind[1].length = new unsigned long;

        resultBind[2].buffer_type = MYSQL_TYPE_STRING;
        resultBind[2].buffer = passwordHash;
        resultBind[2].buffer_length = sizeof(passwordHash);
        resultBind[2].length = new unsigned long;

        resultBind[3].buffer_type = MYSQL_TYPE_STRING;
        resultBind[3].buffer = role;
        resultBind[3].buffer_length = sizeof(role);
        resultBind[3].length = new unsigned long;

        resultBind[4].buffer_type = MYSQL_TYPE_STRING;
        resultBind[4].buffer = createdAt;
        resultBind[4].buffer_length = sizeof(createdAt);
        resultBind[4].length = new unsigned long;

        if (mysql_stmt_bind_result(stmt, resultBind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            // 清理内存
            delete resultBind[1].length;
            delete resultBind[2].length;
            delete resultBind[3].length;
            delete resultBind[4].length;
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定结果失败: " + error);
        }

        // 获取结果
        if (mysql_stmt_fetch(stmt) == 0) {
            // 创建用户对象
            auto user = std::make_shared<User>();
            user->setId(idResult);
            user->setUsername(std::string(username, *resultBind[1].length));
            user->setPasswordHash(std::string(passwordHash, *resultBind[2].length));
            user->setRole(std::string(role, *resultBind[3].length));
            user->setCreatedAt(std::string(createdAt, *resultBind[4].length));

            // 清理内存
            delete resultBind[1].length;
            delete resultBind[2].length;
            delete resultBind[3].length;
            delete resultBind[4].length;
            mysql_stmt_close(stmt);

            return user;
        }

        // 清理内存
        delete resultBind[1].length;
        delete resultBind[2].length;
        delete resultBind[3].length;
        delete resultBind[4].length;
        mysql_stmt_close(stmt);

        return nullptr;
    }
    catch(const std::exception& e)
    {
        spdlog::error("通过ID获取用户失败: {}", e.what());
        return nullptr;
    }
}

std::vector<User> UserRepository::getAllUsers()
{
    std::vector<User> users;
    try
    {
        MYSQL* conn = static_cast<MYSQL*>(db);
        if (!conn) {
            // 数据库连接无效，返回空向量
            spdlog::error("数据库连接无效，无法获取所有用户");
            return users;
        }

        // 执行查询
        std::string sql = "SELECT id, username, password, role, created_at FROM users";
        if (mysql_query(conn, sql.c_str()) != 0) {
            throw std::runtime_error("执行查询失败: " + std::string(mysql_error(conn)));
        }

        // 获取结果集
        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            throw std::runtime_error("获取结果集失败: " + std::string(mysql_error(conn)));
        }

        // 遍历结果
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)) != nullptr) {
            User user;
            user.setId(std::stoi(row[0]));
            user.setUsername(row[1]);
            user.setPasswordHash(row[2]);
            user.setRole(row[3]);
            user.setCreatedAt(row[4]);
            users.push_back(user);
        }

        // 清理
        mysql_free_result(result);
    }
    catch(const std::exception& e)
    {
        spdlog::error("获取所有用户失败: {}", e.what());
    }

    return users;
}

bool UserRepository::updateUser(const User &user)
{
    try
    {
        MYSQL* conn = static_cast<MYSQL*>(db);
        if (!conn) {
            // 数据库连接无效，返回失败
            spdlog::error("数据库连接无效，无法更新用户");
            return false;
        }

        // 准备SQL语句
        std::string sql = "UPDATE users SET username = ?, password = ?, role = ?, updated_at = NOW() WHERE id = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            throw std::runtime_error("无法初始化SQL语句");
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("准备SQL语句失败: " + error);
        }

        // 绑定参数
        MYSQL_BIND bind[4];
        memset(bind, 0, sizeof(bind));

        // username
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)user.getUsername().c_str();
        bind[0].buffer_length = user.getUsername().length();

        // password
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)user.getPasswordHash().c_str();
        bind[1].buffer_length = user.getPasswordHash().length();

        // role
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)user.getRole().c_str();
        bind[2].buffer_length = user.getRole().length();

        // id
        int id = user.getId();
        bind[3].buffer_type = MYSQL_TYPE_LONG;
        bind[3].buffer = &id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定参数失败: " + error);
        }

        // 执行语句
        if (mysql_stmt_execute(stmt) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("执行SQL语句失败: " + error);
        }

        // 检查影响的行数
        my_ulonglong affectedRows = mysql_stmt_affected_rows(stmt);
        mysql_stmt_close(stmt);

        return affectedRows > 0;
    }
    catch(const std::exception& e)
    {
        spdlog::error("更新用户失败: {}", e.what());
        return false;
    }
}

bool UserRepository::deleteUser(int id)
{
    try
    {
        MYSQL* conn = static_cast<MYSQL*>(db);
        if (!conn) {
            // 数据库连接无效，返回失败
            spdlog::error("数据库连接无效，无法删除用户");
            return false;
        }

        // 准备SQL语句
        std::string sql = "DELETE FROM users WHERE id = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            throw std::runtime_error("无法初始化SQL语句");
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("准备SQL语句失败: " + error);
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定参数失败: " + error);
        }

        // 执行语句
        if (mysql_stmt_execute(stmt) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("执行SQL语句失败: " + error);
        }

        // 检查影响的行数
        my_ulonglong affectedRows = mysql_stmt_affected_rows(stmt);
        mysql_stmt_close(stmt);

        return affectedRows > 0;
    }
    catch(const std::exception& e)
    {
        spdlog::error("删除用户失败: {}", e.what());
        return false;
    }
}




