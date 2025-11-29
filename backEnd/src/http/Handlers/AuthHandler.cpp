#include "AuthHandler.h"
#include "Utils/JsonParser.h"
#include "Utils/Constants.h"
#include <spdlog/spdlog.h>
#include <Middleware/AuthMiddleware.h>

AuthHandler::AuthHandler() {}

void AuthHandler::registerRoutes(httplib::Server& svr)
{ 
    svr.Post("/auth/login", [this](const httplib::Request& req, httplib::Response& res)
    {
        this->handleLogin(req, res);
    });

    svr.Post("/auth/logout", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleLogout(req, res);
    });
    
    svr.Post("/auth/refresh", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleRefreshToken(req, res);
    });
    
    svr.Get("/auth/profile", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetProfile(req, res);
    });
}

void AuthHandler::handleLogin(const httplib::Request& req, httplib::Response& res)
{ 
    try
    {
        // 解析请求参数
        auto jsonData = JsonParser::parse(req.body);

        // 验证参数
        if(!jsonData.contains("username") || !jsonData.contains("password"))
        {
            res.status = Constants::RESPONSE_BAD_REQUEST;
            res.set_content(R"({"error": "缺少用户名或密码"})", "application/json");
            return;
        }

        std::string username = jsonData["username"];
        std::string password = jsonData["password"];

        // 调用认证服务进行登录验证
        auto [success, message] = authService.login(username, password);

        if(success)
        {
            // 生成Jwt令牌
            auto user = authService.getUserByUsername(username); 
            std::string token = authService.generateJwtToken(*user);

            // 构建成功响应
            nlohmann::json response;
            response["success"] = true;
            response["token"] = token;
            response["user"] = {
                {"id", user->getId()},
                {"username", user->getUsername()},
                {"role", user->getRole()}
            };

            res.status = Constants::RESPONSE_SUCCESS;
            res.set_content(JsonParser::serialize(response), "application/json");
        }
        else
        {
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            nlohmann::json response;
            response["error"] = message;
            res.set_content(JsonParser::serialize(response), "application/json");
        }
    }
    catch(const std::exception& e)
    {
        spdlog::error("登录处理错误: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
    
}

void AuthHandler::handleLogout(const httplib::Request& req, httplib::Response& res)
{
    res.status = Constants::RESPONSE_SUCCESS;
    res.set_content(R"({"success": true, "message": "登出成功"})", "application/json");
}

void AuthHandler::handleRefreshToken(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // 从头请求或请求体获得旧令牌
        std::string oldToken = AuthMiddleware::extractToken(req);

        if(oldToken.empty())
        {
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            res.set_content(R"({"error": "未提供令牌"})", "application/json");
            return;
        }

        // 刷新令牌
        std::string newToken = authService.refreshJwtToken(oldToken);

        if(newToken.empty())
        {  
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            res.set_content(R"({"error": "令牌无效或已过期"})", "application/json");
            return;
        }

        res.status = Constants::RESPONSE_SUCCESS;
        nlohmann::json response;
        response["success"] = true;
        response["token"] = newToken;
        res.set_content(JsonParser::serialize(response), "application/json");
    }
    catch(const std::exception& e)
    {
        spdlog::error("刷新令牌错误: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void AuthHandler::handleGetProfile(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // 验证令牌
        if(!AuthMiddleware::validateToken(req))
        {
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            res.set_content(R"({"error": "未授权访问"})", "application/json");
            return;
        }

        // 获取当前用户
        std::string token = AuthMiddleware::extractToken(req);
        auto user = authService.getCurrentUser(token);

        if (!user) {
            res.status = Constants::RESPONSE_NOT_FOUND;
            res.set_content(R"({"error": "用户不存在"})", "application/json");
            return;
        }

        // 构建用户信息响应
        nlohmann::json response;
        response["success"] = true;
        response["user"] = {
            {"id", user->getId()},
            {"username", user->getUsername()},
            {"role", user->getRole()},
            {"createdAt", user->getCreatedAt()}
        };

        res.status = Constants::RESPONSE_SUCCESS;
        res.set_content(JsonParser::serialize(response), "application/json");
    }
    catch(const std::exception& e)
    {
        spdlog::error("获取用户信息错误: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}