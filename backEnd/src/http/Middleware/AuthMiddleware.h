#pragma once
#include <httplib.h>
#include "Services/AuthService.h"

class AuthMiddleware {
public:
    // 验证请求中的令牌
    static bool validateToken(const httplib::Request& req);
    
    // 从请求中提取令牌
    static std::string extractToken(const httplib::Request& req);
    
    // 检查用户角色权限
    static bool checkRole(const httplib::Request& req, const std::string& requiredRole);
    
    // 获取当前用户信息
    static std::shared_ptr<User> getCurrentUser(const httplib::Request& req);

private:
    static AuthService authService;
};