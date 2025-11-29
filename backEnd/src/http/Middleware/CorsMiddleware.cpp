#include "CorsMiddleware.h"

void applyCorsMiddleware(httplib::Server& svr)
{ 
    // 设置CORS中间件
    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        // 设置允许的源
        res.set_header("Access-Control-Allow-Origin", "*");
        
        // 设置允许的HTTP方法
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        
        // 设置允许的请求头
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        
        // 设置是否允许携带凭证
        res.set_header("Access-Control-Allow-Credentials", "true");
        
        // 处理预检请求
        if (req.method == "OPTIONS") {
            res.status = 204;
            return httplib::Server::HandlerResponse::Handled;
        }
        
        return httplib::Server::HandlerResponse::Unhandled;
    });
}