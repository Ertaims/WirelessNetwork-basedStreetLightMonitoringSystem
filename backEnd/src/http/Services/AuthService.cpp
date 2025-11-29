#include "AuthService.h"
#include "Utils/Jwt.h"
#include "Utils/Constants.h"
#include <spdlog/spdlog.h>
#include <chrono>

AuthService::AuthService() {};

auto AuthService::login(const std::string &username, const std::string &password)
-> std::pair<bool, std::string>
{
    try
    {
        // 查找用户
        auto user = userRepository.getUserByUsername(username);

        if(!user)
        {
            return {false, "用户不存在"};
        }

        // 验证密码
        if(!user->checkPassword(password))
        {
            return {false, "密码错误"};
        }

        return {true, "登录成功"};
    }
    catch (const std::exception &e)
    {
        spdlog::error("登录验证错误: {}", e.what());
        return {false, "登录验证失败"};
    }
}

std::string AuthService::generateJwtToken(const User &user)
{
    try
    {
        Jwt jwt(jwtSecret);

        // 设置JWT声明
        std::map<std::string, std::string> claims;
        claims["sub"] = std::to_string(user.getId());
        claims["username"] = user.getUsername();
        claims["role"] = user.getRole();
        claims["iat"] = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

        // 生成令牌
        return jwt.generateToken(claims, tokenExpirationTime);
    }
    catch (const std::exception &e)
    {
        spdlog::error("生成令牌错误: {}", e.what());
        return "";
    }
}

bool AuthService::ValidateJwtToken(const std::string & token)
{
    try {
        Jwt jwt(jwtSecret);
        return jwt.validateToken(token);
    } catch (const std::exception& e) {
        spdlog::error("验证令牌错误: {}", e.what());
        return false;
    }
}

std::string AuthService::refreshJwtToken(const std::string& oldToken)
{
    try
    {
        Jwt jwt(jwtSecret);

        // 验证令牌是否有效
        if(!jwt.validateToken(oldToken))
        {
            return "";
        }

        // 提取用户信息
        auto claims = jwt.extractClaims(oldToken);
        if(claims.find("username") == claims.end())
        {
            return "";
        }

        // 获取用户并生成新令牌
        auto user = userRepository.getUserByUsername(claims["username"]);
        if (!user) 
        {
            return "";
        }

        return generateJwtToken(*user);
    }
    catch (const std::exception &e)
    {
        spdlog::error("刷新令牌错误: {}", e.what());
        return "";
    }
}

std::shared_ptr<User> AuthService::getUserByUsername(const std::string &userName)
{
    return userRepository.getUserByUsername(userName);
}

std::shared_ptr<User> AuthService::getCurrentUser(const std::string &token)
{
    try
    {
        Jwt jwt(jwtSecret);
        
        // 验证令牌
        if (!jwt.validateToken(token)) {
            return nullptr;
        }
        
        // 提取用户信息
        auto claims = jwt.extractClaims(token);
        if (claims.find("username") == claims.end()) {
            return nullptr;
        }
        
        // 获取用户
        return userRepository.getUserByUsername(claims["username"]);
    }
    catch(const std::exception& e)
    {
        spdlog::error("获取当前用户错误: {}", e.what());
        return nullptr;
    }
}
