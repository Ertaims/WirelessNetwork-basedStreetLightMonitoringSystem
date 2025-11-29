#pragma once

#include <string>
#include <memory>
#include "Models/User.h"
#include "Repositories/UserRepository.h"

class AuthService
{
public:
    AuthService();

    // 登录验证
    std::pair<bool, std::string> login(const std::string& username, const std::string& password);

    // 生成JWT令牌
    std::string generateJwtToken(const User& user);

    // 验证JWT令牌
    bool ValidateJwtToken(const std::string& token);

    // 刷新令牌
    std::string refreshJwtToken(const std::string& oldToken);

    // 获取用户信息
    std::shared_ptr<User> getUserByUsername(const std::string& userName);

    // 获取当前用户（从令牌中）
    std::shared_ptr<User> getCurrentUser(const std::string& token);

private:
    UserRepository userRepository;
    std::string jwtSecret; // 实际应用中应从配置文件读取
    int tokenExpirationTime; // 令牌过期时间（秒）
};