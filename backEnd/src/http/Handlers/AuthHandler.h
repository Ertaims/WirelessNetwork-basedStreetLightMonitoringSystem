#pragma once
#include <httplib.h>
#include "Services/AuthService.h"

class AuthHandler {
public:
    AuthHandler();
    void registerRoutes(httplib::Server& svr);

private:
    AuthService authService;
    
    // API处理函数
    void handleLogin(const httplib::Request& req, httplib::Response& res);
    void handleLogout(const httplib::Request& req, httplib::Response& res);
    void handleRefreshToken(const httplib::Request& req, httplib::Response& res);
    void handleGetProfile(const httplib::Request& req, httplib::Response& res);
};