#include "LogMiddleware.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <iomanip>
#include <sstream>

void applyLogMiddleware(httplib::Server& svr)
{ 
    // 请求前的日志中间件
    svr.set_post_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        // 记录请求信息
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << "[" << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S") << "] "
           << req.method << " " << req.path << " " 
           << "Status: " << res.status << " "
           << "Content-Type: " << res.get_header_value("Content-Type");
        
        // 根据状态码选择日志级别
        if (res.status >= 500) {
            spdlog::error(ss.str());
        } else if (res.status >= 400) {
            spdlog::warn(ss.str());
        } else {
            spdlog::info(ss.str());
        }
        
        return httplib::Server::HandlerResponse::Unhandled;
    });
}