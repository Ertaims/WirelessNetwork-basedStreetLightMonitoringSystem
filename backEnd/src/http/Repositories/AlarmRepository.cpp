#include "AlarmRepository.h"
#include "config/config.h"
#include <mysql/mysql.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <sstream>

AlarmRepository::AlarmRepository() {}

AlarmRepository::~AlarmRepository() {}

std::vector<Alarm> AlarmRepository::getAlarms(int page, int pageSize, const std::string& status, const std::string& level)
{
    std::vector<Alarm> alarms;
    MYSQL_RES* result = nullptr;
    MYSQL* conn = nullptr;
    
    // 为每个请求创建一个新的连接，使用完毕后关闭，避免多线程共享连接导致的问题
    conn = mysql_init(nullptr);
    if (!conn) {
        spdlog::error("无法初始化MySQL连接");
        return alarms;
    }
    
    try {
        // 从配置中获取数据库连接参数
        Config& config = Config::getInstance();
        std::string host = config.getDatabaseHost();
        std::string user = config.getDatabaseUsername();
        std::string password = config.getDatabasePassword();
        std::string database = config.getDatabaseName();
        int port = config.getDatabasePort();
        
        // 设置连接超时和重试参数
        mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
        mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, "300");
        mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, "300");
        mysql_options(conn, MYSQL_INIT_COMMAND, "SET NAMES utf8mb4");
        mysql_options(conn, MYSQL_INIT_COMMAND, "SET CHARACTER SET utf8mb4");
        
        // 建立连接
        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0)) {
            spdlog::error("连接数据库失败: {}", mysql_error(conn));
            mysql_close(conn);
            return alarms;
        }
        
        // 构建查询语句
        std::stringstream query;
        query << "SELECT * FROM alarms";
        
        bool hasWhere = false;
        if (!status.empty()) {
            query << (hasWhere ? " AND " : " WHERE ") << "status = '" << status << "'";
            hasWhere = true;
        }
        if (!level.empty()) {
            query << (hasWhere ? " AND " : " WHERE ") << "level = '" << level << "'";
            hasWhere = true;
        }
        
        // 按时间倒序排列
        query << " ORDER BY occurred_at DESC";
        
        // 添加分页
        if (page > 0 && pageSize > 0) {
            int offset = (page - 1) * pageSize;
            query << " LIMIT " << pageSize << " OFFSET " << offset;
        }
        
        // 执行查询
        if (mysql_query(conn, query.str().c_str()) != 0) {
            spdlog::error("查询告警失败: {}", mysql_error(conn));
            mysql_close(conn);
            return alarms;
        }
        
        result = mysql_store_result(conn);
        if (result == nullptr) {
            // 检查是否因为查询返回空结果集
            if (mysql_field_count(conn) == 0) {
                // 空结果集，不是错误
                spdlog::info("告警查询返回空结果集");
                mysql_close(conn);
                return alarms;
            } else {
                spdlog::error("获取告警结果失败: {}", mysql_error(conn));
                mysql_close(conn);
                return alarms;
            }
        }
        
        // 获取字段数量，确保访问row时不会越界
        int num_fields = mysql_num_fields(result);
        if (num_fields == 0) {
            spdlog::info("告警结果集字段数为0");
            mysql_free_result(result);
            mysql_close(conn);
            return alarms;
        }
        
        // 遍历结果集
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)) != nullptr) {
            try {
                Alarm alarm;
                
                // 填充告警数据，确保row[i]不为空且是有效数字
                if (row[0] != nullptr && row[0][0] != '\0') {
                    try {
                        alarm.setId(std::stoi(row[0]));
                    } catch (const std::exception& e) {
                        spdlog::error("无效的ID值: {}", row[0]);
                        continue;
                    }
                }
                if (row[1] != nullptr && row[1][0] != '\0') {
                    try {
                        alarm.setDeviceId(std::stoi(row[1]));
                    } catch (const std::exception& e) {
                        spdlog::error("无效的Device ID值: {}", row[1]);
                        continue;
                    }
                }
                alarm.setType(row[2] ? row[2] : "");
                alarm.setLevel(row[3] ? row[3] : "");
                alarm.setStatus(row[4] ? row[4] : "");
                alarm.setDescription(row[5] ? row[5] : "");
                if (row[6] != nullptr && row[6][0] != '\0') {
                    try {
                        alarm.setValue(std::stod(row[6]));
                    } catch (const std::exception& e) {
                        spdlog::error("无效的Value值: {}", row[6]);
                    }
                }
                if (row[7] != nullptr && row[7][0] != '\0') {
                    try {
                        alarm.setThreshold(std::stod(row[7]));
                    } catch (const std::exception& e) {
                        spdlog::error("无效的Threshold值: {}", row[7]);
                    }
                }
                
                // 处理时间字段，确保是有效数字
                if (row[8] != nullptr && row[8][0] != '\0') {
                    try {
                        time_t occurred_at = std::stol(row[8]);
                        alarm.setOccurredAt(occurred_at);
                    } catch (const std::exception& e) {
                        spdlog::error("无效的Occurred At值: {}", row[8]);
                    }
                }
                if (row[9] != nullptr && row[9][0] != '\0') {
                    try {
                        time_t confirmed_at = std::stol(row[9]);
                        alarm.setConfirmedAt(confirmed_at);
                    } catch (const std::exception& e) {
                        spdlog::error("无效的Confirmed At值: {}", row[9]);
                    }
                }
                if (row[10] != nullptr && row[10][0] != '\0') {
                    try {
                        time_t resolved_at = std::stol(row[10]);
                        alarm.setResolvedAt(resolved_at);
                    } catch (const std::exception& e) {
                        spdlog::error("无效的Resolved At值: {}", row[10]);
                    }
                }
                
                alarm.setRemark(row[11] ? row[11] : "");
                alarm.setConfirmedBy(row[12] ? row[12] : "");
                alarm.setResolvedBy(row[13] ? row[13] : "");
                
                alarms.push_back(alarm);
            } catch (const std::exception& e) {
                spdlog::error("处理告警数据失败: {}", e.what());
                continue;
            }
        }
        
        // 释放结果集
        mysql_free_result(result);
        
        spdlog::info("获取告警成功，共 {} 个告警", alarms.size());
    } catch (const std::exception& e) {
        spdlog::error("获取告警异常: {}", e.what());
        // 确保结果集和连接被释放
        if (result != nullptr) {
            mysql_free_result(result);
        }
        if (conn != nullptr) {
            mysql_close(conn);
        }
        return alarms;
    }
    
    // 关闭连接
    if (conn != nullptr) {
        mysql_close(conn);
    }
    
    return alarms;
}

std::vector<std::pair<std::string, int>> AlarmRepository::getAlarmStats()
{
    std::vector<std::pair<std::string, int>> stats;
    MYSQL_RES* result = nullptr;
    MYSQL* conn = nullptr;
    
    // 为每个请求创建一个新的连接，使用完毕后关闭，避免多线程共享连接导致的问题
    conn = mysql_init(nullptr);
    if (!conn) {
        spdlog::error("无法初始化MySQL连接");
        return stats;
    }
    
    try {
        // 从配置中获取数据库连接参数
        Config& config = Config::getInstance();
        std::string host = config.getDatabaseHost();
        std::string user = config.getDatabaseUsername();
        std::string password = config.getDatabasePassword();
        std::string database = config.getDatabaseName();
        int port = config.getDatabasePort();
        
        // 设置连接超时和重试参数
        mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
        mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, "300");
        mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, "300");
        mysql_options(conn, MYSQL_INIT_COMMAND, "SET NAMES utf8mb4");
        mysql_options(conn, MYSQL_INIT_COMMAND, "SET CHARACTER SET utf8mb4");
        
        // 建立连接
        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0)) {
            spdlog::error("连接数据库失败: {}", mysql_error(conn));
            mysql_close(conn);
            return stats;
        }
        
        // 查询告警统计数据
        const char* query = "SELECT status, COUNT(*) as count FROM alarms GROUP BY status";
        
        if (mysql_query(conn, query) != 0) {
            spdlog::error("查询告警统计失败: {}", mysql_error(conn));
            mysql_close(conn);
            return stats;
        }
        
        result = mysql_store_result(conn);
        if (result == nullptr) {
            // 检查是否因为查询返回空结果集
            if (mysql_field_count(conn) == 0) {
                // 空结果集，不是错误
                spdlog::info("告警统计查询返回空结果集");
                mysql_close(conn);
                return stats;
            } else {
                spdlog::error("获取告警统计结果失败: {}", mysql_error(conn));
                mysql_close(conn);
                return stats;
            }
        }
        
        // 获取字段数量，确保访问row时不会越界
        int num_fields = mysql_num_fields(result);
        if (num_fields < 2) {
            spdlog::info("告警统计结果集字段数不足");
            mysql_free_result(result);
            mysql_close(conn);
            return stats;
        }
        
        // 遍历结果集
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)) != nullptr) {
            try {
                if (row[0] != nullptr && row[1] != nullptr) {
                    std::string status = row[0];
                    // 确保row[1]是有效数字
                    std::string count_str = row[1];
                    bool is_digit = true;
                    for (char c : count_str) {
                        if (!std::isdigit(c)) {
                            is_digit = false;
                            break;
                        }
                    }
                    if (is_digit && !count_str.empty()) {
                        int count = std::stoi(count_str);
                        stats.emplace_back(status, count);
                    } else {
                        spdlog::warn("跳过无效的告警统计行: 状态={}, 数量={}", row[0], count_str);
                    }
                }
            } catch (const std::exception& e) {
                spdlog::error("处理告警统计数据失败: {}", e.what());
                continue;
            }
        }
        
        // 释放结果集
        mysql_free_result(result);
        
        // 关闭连接
        mysql_close(conn);
        
        spdlog::info("获取告警统计成功");
    } catch (const std::exception& e) {
        spdlog::error("获取告警统计异常: {}", e.what());
        // 确保结果集和连接被释放
        if (result != nullptr) {
            mysql_free_result(result);
        }
        if (conn != nullptr) {
            mysql_close(conn);
        }
    }
    
    return stats;
}

bool AlarmRepository::confirmAlarm(int id, const std::string& remark, const std::string& confirmed_by)
{
    // 为每个请求创建一个新的连接，使用完毕后关闭，避免多线程共享连接导致的问题
    MYSQL* conn = nullptr;
    
    conn = mysql_init(nullptr);
    if (!conn) {
        spdlog::error("无法初始化MySQL连接");
        return false;
    }
    
    try {
        // 从配置中获取数据库连接参数
        Config& config = Config::getInstance();
        std::string host = config.getDatabaseHost();
        std::string user = config.getDatabaseUsername();
        std::string password = config.getDatabasePassword();
        std::string database = config.getDatabaseName();
        int port = config.getDatabasePort();
        
        // 设置连接超时和重试参数
        mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
        
        // 建立连接
        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0)) {
            spdlog::error("连接数据库失败: {}", mysql_error(conn));
            mysql_close(conn);
            return false;
        }
        
        // 构建更新语句
        std::stringstream query;
        query << "UPDATE alarms SET status = 'CONFIRMED', confirmed_at = NOW()"
              << ", remark = '" << remark << "', confirmed_by = '" << confirmed_by
              << "' WHERE id = " << id;
        
        // 执行更新
        if (mysql_query(conn, query.str().c_str()) != 0) {
            spdlog::error("确认告警失败: {}", mysql_error(conn));
            mysql_close(conn);
            return false;
        }
        
        // 检查影响的行数
        my_ulonglong affected_rows = mysql_affected_rows(conn);
        
        // 关闭连接
        mysql_close(conn);
        
        return affected_rows > 0;
    } catch (const std::exception& e) {
        spdlog::error("确认告警异常: {}", e.what());
        if (conn != nullptr) {
            mysql_close(conn);
        }
        return false;
    }
}

bool AlarmRepository::resolveAlarm(int id, const std::string& remark, const std::string& resolved_by)
{
    // 为每个请求创建一个新的连接，使用完毕后关闭，避免多线程共享连接导致的问题
    MYSQL* conn = nullptr;
    
    conn = mysql_init(nullptr);
    if (!conn) {
        spdlog::error("无法初始化MySQL连接");
        return false;
    }
    
    try {
        // 从配置中获取数据库连接参数
        Config& config = Config::getInstance();
        std::string host = config.getDatabaseHost();
        std::string user = config.getDatabaseUsername();
        std::string password = config.getDatabasePassword();
        std::string database = config.getDatabaseName();
        int port = config.getDatabasePort();
        
        // 设置连接超时和重试参数
        mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
        
        // 建立连接
        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0)) {
            spdlog::error("连接数据库失败: {}", mysql_error(conn));
            mysql_close(conn);
            return false;
        }
        
        // 首先获取告警信息，包括关联的设备ID和设备当前状态
        std::stringstream selectQuery;
        selectQuery << "SELECT a.device_id, d.status FROM alarms a JOIN devices d ON a.device_id = d.id WHERE a.id = " << id;
        
        if (mysql_query(conn, selectQuery.str().c_str()) != 0) {
            spdlog::error("查询告警和设备信息失败: {}", mysql_error(conn));
            mysql_close(conn);
            return false;
        }
        
        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            spdlog::error("获取查询结果失败: {}", mysql_error(conn));
            mysql_close(conn);
            return false;
        }
        
        MYSQL_ROW row = mysql_fetch_row(result);
        int deviceId = -1;
        std::string deviceStatus;
        
        if (row) {
            if (row[0]) deviceId = std::stoi(row[0]);
            if (row[1]) deviceStatus = row[1];
        }
        
        mysql_free_result(result);
        
        // 开始事务
        if (mysql_query(conn, "START TRANSACTION") != 0) {
            spdlog::error("开启事务失败: {}", mysql_error(conn));
            mysql_close(conn);
            return false;
        }
        
        // 构建更新语句
        std::stringstream updateQuery;
        updateQuery << "UPDATE alarms SET status = 'RESOLVED', resolved_at = NOW()"
              << ", remark = '" << remark << "', resolved_by = '" << resolved_by
              << "' WHERE id = " << id;
        
        // 执行更新告警
        if (mysql_query(conn, updateQuery.str().c_str()) != 0) {
            spdlog::error("解决告警失败: {}", mysql_error(conn));
            mysql_query(conn, "ROLLBACK");
            mysql_close(conn);
            return false;
        }
        
        // 检查影响的行数
        my_ulonglong affected_rows = mysql_affected_rows(conn);
        
        // 如果解决了告警并且设备状态是FAULT，则更新设备状态为ONLINE
        if (affected_rows > 0 && deviceId != -1 && deviceStatus == "FAULT") {
            std::stringstream deviceUpdateQuery;
            deviceUpdateQuery << "UPDATE devices SET status = 'ONLINE' WHERE id = " << deviceId;
            
            if (mysql_query(conn, deviceUpdateQuery.str().c_str()) != 0) {
                spdlog::error("更新设备状态失败: {}", mysql_error(conn));
                mysql_query(conn, "ROLLBACK");
                mysql_close(conn);
                return false;
            }
        }
        
        // 提交事务
        if (mysql_query(conn, "COMMIT") != 0) {
            spdlog::error("提交事务失败: {}", mysql_error(conn));
            mysql_query(conn, "ROLLBACK");
            mysql_close(conn);
            return false;
        }
        
        // 关闭连接
        mysql_close(conn);
        
        return affected_rows > 0;
    } catch (const std::exception& e) {
        spdlog::error("解决告警异常: {}", e.what());
        if (conn != nullptr) {
            mysql_close(conn);
        }
        return false;
    }
}

bool AlarmRepository::batchProcessAlarms(const std::vector<int>& alarm_ids, const std::string& action, const std::string& remark, const std::string& operator_name)
{
    // 为每个请求创建一个新的连接，使用完毕后关闭，避免多线程共享连接导致的问题
    MYSQL* conn = nullptr;
    
    conn = mysql_init(nullptr);
    if (!conn) {
        spdlog::error("无法初始化MySQL连接");
        return false;
    }
    
    try {
        // 从配置中获取数据库连接参数
        Config& config = Config::getInstance();
        std::string host = config.getDatabaseHost();
        std::string user = config.getDatabaseUsername();
        std::string password = config.getDatabasePassword();
        std::string database = config.getDatabaseName();
        int port = config.getDatabasePort();
        
        // 设置连接超时和重试参数
        mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
        
        // 建立连接
        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0)) {
            spdlog::error("连接数据库失败: {}", mysql_error(conn));
            mysql_close(conn);
            return false;
        }
        
        // 构建ID列表
        std::stringstream id_list;
        for (size_t i = 0; i < alarm_ids.size(); i++) {
            if (i > 0) {
                id_list << ",";
            }
            id_list << alarm_ids[i];
        }
        
        // 构建更新语句
        std::stringstream query;
        query << "UPDATE alarms SET remark = '" << remark << "'";
        
        if (action == "confirm") {
            query << ", status = 'CONFIRMED', confirmed_at = NOW()"
                  << ", confirmed_by = '" << operator_name << "'";
        } else if (action == "resolve") {
            query << ", status = 'RESOLVED', resolved_at = NOW()"
                  << ", resolved_by = '" << operator_name << "'";
        } else {
            spdlog::error("无效的批量操作类型: {}", action);
            mysql_close(conn);
            return false;
        }
        
        query << " WHERE id IN (" << id_list.str() << ")";
        
        // 执行更新
        if (mysql_query(conn, query.str().c_str()) != 0) {
            spdlog::error("批量处理告警失败: {}", mysql_error(conn));
            mysql_close(conn);
            return false;
        }
        
        // 检查影响的行数
        my_ulonglong affected_rows = mysql_affected_rows(conn);
        
        // 关闭连接
        mysql_close(conn);
        
        return affected_rows > 0;
    } catch (const std::exception& e) {
        spdlog::error("批量处理告警异常: {}", e.what());
        if (conn != nullptr) {
            mysql_close(conn);
        }
        return false;
    }
}
