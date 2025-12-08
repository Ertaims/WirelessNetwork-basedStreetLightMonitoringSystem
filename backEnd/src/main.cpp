#include "core/LampMonitor.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <iostream>
#include <mysql/mysql.h>

#include "config/config.h"
#include "http/Handlers/AuthHandler.h"
#include "http/Handlers/DeviceHandler.h"
#include "http/Handlers/AlarmHandler.h"
#include "http/Middleware/AuthMiddleware.h"
#include "http/Middleware/CorsMiddleware.h"
#include "http/Middleware/LogMiddleware.h"

int main() {
    // 初始化日志
    spdlog::set_level(spdlog::level::info);
    spdlog::info("智能路灯监控系统后端启动...");

    // 初始化并启动LampMonitor
    spdlog::info("初始化LampMonitor...");
    const std::string server_address = "tcp://10.201.40.164:1883";
    const std::string client_id = "LampMonitorClient";
    
    LampMonitor& lampMonitor = LampMonitor::getInstance();
    if (!lampMonitor.initialize(server_address, client_id)) {
        spdlog::error("无法初始化LampMonitor");
    } else if (!lampMonitor.start()) {
        spdlog::error("无法启动LampMonitor");
    } else {
        spdlog::info("LampMonitor启动成功");
    }

    // 初始化MySQL库
    mysql_library_init(0, nullptr, nullptr);

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

    spdlog::info("创建DeviceHandler");
    DeviceHandler deviceHandler;
    deviceHandler.registerRoutes(svr);

    spdlog::info("创建AlarmHandler");
    AlarmHandler alarmHandler;
    alarmHandler.registerRoutes(svr);

    UserRepository userRepository;
    
    // 先尝试获取并删除现有用户
    auto existingUser = userRepository.getUserByUsername("user");
    if (existingUser) {
        userRepository.deleteUser(existingUser->getId());
    }
    existingUser = userRepository.getUserByUsername("admin");
    if (existingUser) {
        userRepository.deleteUser(existingUser->getId());
    }
    existingUser = userRepository.getUserByUsername("viewer");
    if (existingUser) {
        userRepository.deleteUser(existingUser->getId());
    }
    
    // 重新创建用户
    User user("user", "123456", "OPERATOR", "测试用户", "user@example.com", "user123456");
    User admin("admin", "123456", "ADMIN", "管理员", "admin@example.com", "admin123456");
    User viewer("viewer", "123456", "VIEWER", "浏览者", "viewer@example.com", "viewer123456");
    
    userRepository.createUser(user);
    userRepository.createUser(admin);
    userRepository.createUser(viewer);

    // 启动服务器
    int port = 8080;
    spdlog::info("服务器启动在端口 {}", port);
    
    // 启动服务器（阻塞调用）
    svr.listen("0.0.0.0", port);
    
    // 清理MySQL库资源
    mysql_library_end();
    
    return 0;

}