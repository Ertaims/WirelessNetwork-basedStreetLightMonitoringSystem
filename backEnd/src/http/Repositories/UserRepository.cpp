#include "UserRepository.h"
#include "Utils/PasswordHasher.h"
#include "config/config.h"
#include <mysql/mysql.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

UserRepository::UserRepository()
{
    // 不再需要自己初始化数据库连接
}

UserRepository::~UserRepository()
{
    // 不再需要自己关闭数据库连接
}

bool UserRepository::createUser(const User &user)
{
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
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
        
        // 存储字符串长度
        unsigned long lengths[6];
        
        // 为每个字符串创建足够大的缓冲区
        char usernameBuffer[256];
        char passwordBuffer[256];
        char nameBuffer[256];
        char roleBuffer[256];
        char emailBuffer[256];
        char phoneBuffer[256];
        
        // 复制字符串到缓冲区
        strncpy(usernameBuffer, user.getUsername().c_str(), sizeof(usernameBuffer) - 1);
        strncpy(passwordBuffer, user.getPasswordHash().c_str(), sizeof(passwordBuffer) - 1);
        strncpy(nameBuffer, user.getName().c_str(), sizeof(nameBuffer) - 1);
        strncpy(roleBuffer, user.getRole().c_str(), sizeof(roleBuffer) - 1);
        strncpy(emailBuffer, user.getEmail().c_str(), sizeof(emailBuffer) - 1);
        strncpy(phoneBuffer, user.getPhone().c_str(), sizeof(phoneBuffer) - 1);
        
        // 确保字符串以null结尾
        usernameBuffer[sizeof(usernameBuffer) - 1] = '\0';
        passwordBuffer[sizeof(passwordBuffer) - 1] = '\0';
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        roleBuffer[sizeof(roleBuffer) - 1] = '\0';
        emailBuffer[sizeof(emailBuffer) - 1] = '\0';
        phoneBuffer[sizeof(phoneBuffer) - 1] = '\0';

        // username
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = usernameBuffer;
        bind[0].buffer_length = sizeof(usernameBuffer);
        bind[0].length = &lengths[0];
        lengths[0] = user.getUsername().length();

        // password
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = passwordBuffer;
        bind[1].buffer_length = sizeof(passwordBuffer);
        bind[1].length = &lengths[1];
        lengths[1] = user.getPasswordHash().length();

        // name
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = nameBuffer;
        bind[2].buffer_length = sizeof(nameBuffer);
        bind[2].length = &lengths[2];
        lengths[2] = user.getName().length();

        // role
        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = roleBuffer;
        bind[3].buffer_length = sizeof(roleBuffer);
        bind[3].length = &lengths[3];
        lengths[3] = user.getRole().length();

        // email
        bind[4].buffer_type = MYSQL_TYPE_STRING;
        bind[4].buffer = emailBuffer;
        bind[4].buffer_length = sizeof(emailBuffer);
        bind[4].length = &lengths[4];
        lengths[4] = user.getEmail().length();

        // phone
        bind[5].buffer_type = MYSQL_TYPE_STRING;
        bind[5].buffer = phoneBuffer;
        bind[5].buffer_length = sizeof(phoneBuffer);
        bind[5].length = &lengths[5];
        lengths[5] = user.getPhone().length();

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
    MYSQL_STMT* stmt = nullptr;
    unsigned long* length1 = nullptr;
    unsigned long* length2 = nullptr;
    unsigned long* length3 = nullptr;
    unsigned long* length4 = nullptr;
    
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
            return nullptr;
        }

        // 准备SQL语句
        std::string sql = "SELECT id, username, password, role, created_at FROM users WHERE username = ?";
        stmt = mysql_stmt_init(conn);
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
        length1 = new unsigned long;
        resultBind[1].length = length1;

        resultBind[2].buffer_type = MYSQL_TYPE_STRING;
        resultBind[2].buffer = passwordHash;
        resultBind[2].buffer_length = sizeof(passwordHash);
        length2 = new unsigned long;
        resultBind[2].length = length2;

        resultBind[3].buffer_type = MYSQL_TYPE_STRING;
        resultBind[3].buffer = role;
        resultBind[3].buffer_length = sizeof(role);
        length3 = new unsigned long;
        resultBind[3].length = length3;

        resultBind[4].buffer_type = MYSQL_TYPE_STRING;
        resultBind[4].buffer = createdAt;
        resultBind[4].buffer_length = sizeof(createdAt);
        length4 = new unsigned long;
        resultBind[4].length = length4;

        if (mysql_stmt_bind_result(stmt, resultBind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            throw std::runtime_error("绑定结果失败: " + error);
        }

        // 获取结果
        if (mysql_stmt_fetch(stmt) == 0) {
            // 创建用户对象
            auto user = std::make_shared<User>();
            user->setId(id);
            user->setUsername(std::string(usernameResult, *length1));
            user->setPasswordHash(std::string(passwordHash, *length2));
            user->setRole(std::string(role, *length3));
            user->setCreatedAt(std::string(createdAt, *length4));

            // 输出结果
            spdlog::info("获取用户成功: {}", user->getUsername());

            // 清理内存
            delete length1;
            delete length2;
            delete length3;
            delete length4;
            mysql_stmt_close(stmt);

            return user;
        }

        // 清理内存
        delete length1;
        delete length2;
        delete length3;
        delete length4;
        mysql_stmt_close(stmt);

        return nullptr;
    }
    catch(const std::exception& e)
    {
        spdlog::error("通过用户名获取用户失败: {}", e.what());
        // 确保在异常情况下释放所有资源
        delete length1;
        delete length2;
        delete length3;
        delete length4;
        if (stmt) {
            mysql_stmt_close(stmt);
        }
        return nullptr;
    }
}

std::shared_ptr<User> UserRepository::getUserById(int id)
{
    MYSQL_STMT* stmt = nullptr;
    unsigned long* length1 = nullptr;
    unsigned long* length2 = nullptr;
    unsigned long* length3 = nullptr;
    unsigned long* length4 = nullptr;
    
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
            return nullptr;
        }

        // 准备SQL语句
        std::string sql = "SELECT id, username, password, role, created_at FROM users WHERE id = ?";
        stmt = mysql_stmt_init(conn);
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
        length1 = new unsigned long;
        resultBind[1].length = length1;

        resultBind[2].buffer_type = MYSQL_TYPE_STRING;
        resultBind[2].buffer = passwordHash;
        resultBind[2].buffer_length = sizeof(passwordHash);
        length2 = new unsigned long;
        resultBind[2].length = length2;

        resultBind[3].buffer_type = MYSQL_TYPE_STRING;
        resultBind[3].buffer = role;
        resultBind[3].buffer_length = sizeof(role);
        length3 = new unsigned long;
        resultBind[3].length = length3;

        resultBind[4].buffer_type = MYSQL_TYPE_STRING;
        resultBind[4].buffer = createdAt;
        resultBind[4].buffer_length = sizeof(createdAt);
        length4 = new unsigned long;
        resultBind[4].length = length4;

        if (mysql_stmt_bind_result(stmt, resultBind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            throw std::runtime_error("绑定结果失败: " + error);
        }

        // 获取结果
        if (mysql_stmt_fetch(stmt) == 0) {
            // 创建用户对象
            auto user = std::make_shared<User>();
            user->setId(idResult);
            user->setUsername(std::string(username, *length1));
            user->setPasswordHash(std::string(passwordHash, *length2));
            user->setRole(std::string(role, *length3));
            user->setCreatedAt(std::string(createdAt, *length4));

            // 清理内存
            delete length1;
            delete length2;
            delete length3;
            delete length4;
            mysql_stmt_close(stmt);

            return user;
        }

        // 清理内存
        delete length1;
        delete length2;
        delete length3;
        delete length4;
        mysql_stmt_close(stmt);

        return nullptr;
    }
    catch(const std::exception& e)
    {
        spdlog::error("通过ID获取用户失败: {}", e.what());
        // 确保在异常情况下释放所有资源
        delete length1;
        delete length2;
        delete length3;
        delete length4;
        if (stmt) {
            mysql_stmt_close(stmt);
        }
        return nullptr;
    }
}

std::vector<User> UserRepository::getAllUsers()
{
    std::vector<User> users;
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
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
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
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
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
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




