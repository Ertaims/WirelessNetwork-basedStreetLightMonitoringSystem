#pragma once

#include <string>
#include <memory>
#include <vector>
#include <httplib.h>
#include "../Repositories/AlarmRepository.h"

class AlarmHandler
{
public:
    AlarmHandler();
    ~AlarmHandler();

    // 注册路由
    void registerRoutes(httplib::Server& svr);

private:
    // 处理获取告警列表
    void handleGetAlarms(const httplib::Request& req, httplib::Response& res);
    
    // 处理获取告警统计
    void handleGetAlarmStats(const httplib::Request& req, httplib::Response& res);
    
    // 处理确认告警
    void handleConfirmAlarm(const httplib::Request& req, httplib::Response& res);
    
    // 处理解决告警
    void handleResolveAlarm(const httplib::Request& req, httplib::Response& res);
    
    // 处理批量处理告警
    void handleBatchProcessAlarms(const httplib::Request& req, httplib::Response& res);

    // 告警仓库实例
    std::unique_ptr<AlarmRepository> alarmRepo;
};
