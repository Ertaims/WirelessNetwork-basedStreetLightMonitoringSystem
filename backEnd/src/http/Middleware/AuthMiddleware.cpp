#include "AuthMiddleware.h"
#include "Utils/Jwt.h"
#include "Utils/Constants.h"

AuthService AuthMiddleware::authService;

bool AuthMiddleware::validateToken(const httplib::Request &req)
{
    std::string token = extractToken(req);
    if (token.empty()) {
        return false;
    }
    return authService.ValidateJwtToken(token);
}

bool AuthMiddleware::checkRole(const httplib::Request &req, const std::string &requiredRole)
{
    // 获取当前用户
    auto user = getCurrentUser(req);
    if (!user) {
        return false;
    }
    
    // 管理员拥有所有权限
    if (user->getRole() == Constants::USER_ROLE_ADMIN) {
        return true;
    }
    
    // 检查角色
    return user->getRole() == requiredRole;
}

std::shared_ptr<User> AuthMiddleware::getCurrentUser(const httplib::Request &req)
{
    std::string token = extractToken(req);
    if (token.empty()) {
        return nullptr;
    }
    
    return authService.getCurrentUser(token);
}

std::string AuthMiddleware::extractToken(const httplib::Request &req)
{
    // 首先尝试从Authorization头中获取
    if (req.has_header("Authorization")) {
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.substr(0, 7) == "Bearer ") {
            return authHeader.substr(7);
        }
    }
    
    // 然后尝试从查询参数中获取
    if (req.has_param("token")) {
        return req.get_param_value("token");
    }
    
    return "";
}
