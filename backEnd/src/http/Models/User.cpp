#include "User.h"
#include "Utils/PasswordHasher.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

User::User() : id(0) {}

User::User(const std::string& username, const std::string& passwordHash, const std::string& role) 
    : id(0), username(username), passwordHash(passwordHash), role(role) 
{
    // 设置创建时间
    auto now = std::chrono::system_clock::now();                        // 获取当前时间点
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);      // 转换为time_t类型
    std::tm* now_tm = std::localtime(&now_c);                           // 转换为本地时间结构体
    std::ostringstream oss;                                             // 创建字符串流
    oss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");                  // 格式化时间
    created_at = oss.str();
}

int User::getId() const
{
    return id; 
}
void User::setId(int id) 
{
    this->id = id; 
}

std::string User::getUsername() const 
{
    return username; 
}
void User::setUsername(const std::string& username) 
{
    this->username = username; 
}

std::string User::getPasswordHash() const 
{
    return passwordHash; 
}
void User::setPasswordHash(const std::string& passwordHash) 
{
    this->passwordHash = passwordHash; 
}

std::string User::getRole() const 
{
    return role; 
}
void User::setRole(const std::string& role) 
{
    this->role = role; 
}

std::string User::getName() const 
{
    return name; 
}
void User::setName(const std::string& name) 
{
    this->name = name; 
}

std::string User::getEmail() const 
{
    return email; 
}
void User::setEmail(const std::string& email) 
{
    this->email = email; 
}

std::string User::getPhone() const 
{
    return phone; 
}
void User::setPhone(const std::string& phone) 
{
    this->phone = phone; 
}

std::string User::getCreatedAt() const 
{
    return created_at; 
}
void User::setCreatedAt(const std::string& createdAt) 
{
    this->created_at = createdAt; 
}

bool User::checkPassword(const std::string& password) const 
{
    return PasswordHasher::verifyPassword(password, passwordHash);
}