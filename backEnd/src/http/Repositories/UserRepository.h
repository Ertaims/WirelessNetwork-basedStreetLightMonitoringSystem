#pragma once

#include <string>
#include <memory>
#include <vector>
#include "../Models/User.h"

class UserRepository
{
public:
    UserRepository();
    ~UserRepository();
    
    // 创建用户
    bool createUser(const User& user);
    
    // 通过用户名查找用户
    std::shared_ptr<User> getUserByUsername(const std::string& username);
    
    // 通过ID查找用户
    std::shared_ptr<User> getUserById(int id);
    
    // 获取所有用户
    std::vector<User> getAllUsers();
    
    // 更新用户信息
    bool updateUser(const User& user);
    
    // 删除用户
    bool deleteUser(int id);

private:
    // 初始化数据库连接
    void initDatabase();
    
    // 数据库连接句柄
    void* db;
};