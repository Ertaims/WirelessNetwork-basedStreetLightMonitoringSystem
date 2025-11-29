//#include "core/LampMonitor.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <iostream>

#include "config/config.h"
#include "http/Handlers/AuthHandler.h"
#include "http/Middleware/AuthMiddleware.h"
#include "http/Middleware/CorsMiddleware.h"
#include "http/Middleware/LogMiddleware.h"

int main() {
    // // EMQX服务器地址和客户端ID
    // const std::string server_address = "tcp://10.201.40.164:1883";
    // const std::string client_id = "LampMonitorClient";
    
    // std::cout << "启动LampMonitor..." << std::endl;
    // LampMonitor lampMonitor(server_address, client_id);


    // if(!lampMonitor.start()) {
    //     std::cerr << "无法启动LampMonitor." << std::endl;
    //     return -1;
    // }

    // // 模拟路灯控制
    // std::cout << "等待5秒后开始模拟控制..." << std::endl;
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // std::cout << "======= 开始测试路灯控制 =======" << std::endl;
    // // 模拟控制命令
    // lampMonitor.controlLamp("switch", "ON", 'A', "lamp_001", 100);
    // std::this_thread::sleep_for(std::chrono::seconds(2));
    // std::cout << "===============================" << std::endl;
    
    // lampMonitor.controlLamp("switch", "OFF", 'A', "lamp_002");
    // std::this_thread::sleep_for(std::chrono::seconds(2));

    // // lampMonitor.controlLamp("lamp_001", "OFF");

    // // 保持程序运行
    // std::cin.get();

    // std::cout << "====== 暂停客户端连接 ======" << std::endl;
    // lampMonitor.stop();
    // std::cout << "===========================" << std::endl;
    // return 0;

/***********************************************************************/

    // 初始化日志
    spdlog::set_level(spdlog::level::info);
    spdlog::info("智能路灯监控系统后端启动...");

    // 创建HTTP服务器
    httplib::Server svr;
    svr.set_mount_point("/data", "./data");

    // 应用中间件
    applyCorsMiddleware(svr);
    applyLogMiddleware(svr);

    // 注册路由
    spdlog::info("创建AuthHandler");
    AuthHandler authHandler;
    authHandler.registerRoutes(svr);

    // 认证中间件应用于需要保护的路由组
    svr.Post("/api/protected", [](const httplib::Request& req, httplib::Response& res) {
        if (!AuthMiddleware::validateToken(req)) {
            res.status = 401;
            res.set_content(R"({"error": "未授权访问"})", "application/json");
            return;
        }
        res.set_content(R"({"message": "访问成功"})", "application/json");
    });

    // 启动服务器
    int port = 8080;
    spdlog::info("服务器启动在端口 {}", port);
    svr.listen("0.0.0.0", port);

    return 0;

}