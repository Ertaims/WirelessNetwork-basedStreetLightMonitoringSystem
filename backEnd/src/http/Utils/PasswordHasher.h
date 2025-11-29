#pragma once
#include <string>

class PasswordHasher {
public:
    // 加密密码
    static std::string hashPassword(const std::string& password);
    
    // 验证密码
    static bool verifyPassword(const std::string& password, const std::string& hash);
    
    // 生成随机盐值
    static std::string generateSalt(size_t length = 16);

private:
    // 指定盐值加密密码
    static std::string hashPasswordWithSalt(const std::string& password, const std::string& salt);
};