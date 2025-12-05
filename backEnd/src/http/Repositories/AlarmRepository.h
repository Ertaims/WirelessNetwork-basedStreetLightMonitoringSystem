#pragma once

#include <string>
#include <memory>
#include <vector>
#include "Models/Alarm.h"
#include "Utils/DatabaseConnection.h"

class AlarmRepository
{
public:
    AlarmRepository();
    ~AlarmRepository();
    
    // 获取告警列表
    std::vector<Alarm> getAlarms(int page = 1, int pageSize = 20, const std::string& status = "", const std::string& level = "");
    
    // 获取告警统计
    std::vector<std::pair<std::string, int>> getAlarmStats();
    
    // 确认告警
    bool confirmAlarm(int id, const std::string& remark, const std::string& confirmed_by);
    
    // 解决告警
    bool resolveAlarm(int id, const std::string& remark, const std::string& resolved_by);
    
    // 批量处理告警
    bool batchProcessAlarms(const std::vector<int>& alarm_ids, const std::string& action, const std::string& remark, const std::string& operator_name);

private:
    // 数据库连接
    DatabaseConnection dbConnection;
};
