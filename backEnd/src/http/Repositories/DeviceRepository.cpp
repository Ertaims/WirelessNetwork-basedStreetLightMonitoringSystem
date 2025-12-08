#include "DeviceRepository.h"
#include "config/config.h"
#include "core/LampMonitor.h"
#include <mysql/mysql.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

DeviceRepository::DeviceRepository() {}

DeviceRepository::~DeviceRepository() {}

bool DeviceRepository::createDevice(const Device& device)
{
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
            return false;
        }

        // 准备SQL语句
        std::string sql = "INSERT INTO devices (name, location, status, power, brightness, voltage, current, group_name, latitude, longitude, auto_mode, voltage_threshold, current_threshold, on_time, off_time, uptime_hours, last_maintenance, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NULL, NOW(), NOW())";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            throw std::runtime_error("无法初始化SQL语句");
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("准备SQL语句失败: " + error);
        }

        // 绑定参数
        MYSQL_BIND bind[17];
        memset(bind, 0, sizeof(bind));
        
        // 存储字符串长度
        unsigned long lengths[17];
        
        // 为每个字符串创建足够大的缓冲区
        char nameBuffer[256];
        char locationBuffer[256];
        char statusBuffer[256];
        char powerBuffer[256];
        char groupNameBuffer[256];
        char onTimeBuffer[256];
        char offTimeBuffer[256];
        
        // 复制字符串到缓冲区
        strncpy(nameBuffer, device.getName().c_str(), sizeof(nameBuffer) - 1);
        strncpy(locationBuffer, device.getLocation().c_str(), sizeof(locationBuffer) - 1);
        strncpy(statusBuffer, device.getStatus().c_str(), sizeof(statusBuffer) - 1);
        strncpy(powerBuffer, device.getPower().c_str(), sizeof(powerBuffer) - 1);
        strncpy(groupNameBuffer, device.getGroupName().c_str(), sizeof(groupNameBuffer) - 1);
        strncpy(onTimeBuffer, device.getOnTime().c_str(), sizeof(onTimeBuffer) - 1);
        strncpy(offTimeBuffer, device.getOffTime().c_str(), sizeof(offTimeBuffer) - 1);
        
        // 确保字符串以null结尾
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        locationBuffer[sizeof(locationBuffer) - 1] = '\0';
        statusBuffer[sizeof(statusBuffer) - 1] = '\0';
        powerBuffer[sizeof(powerBuffer) - 1] = '\0';
        groupNameBuffer[sizeof(groupNameBuffer) - 1] = '\0';
        onTimeBuffer[sizeof(onTimeBuffer) - 1] = '\0';
        offTimeBuffer[sizeof(offTimeBuffer) - 1] = '\0';

        // name
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = nameBuffer;
        bind[0].buffer_length = sizeof(nameBuffer);
        bind[0].length = &lengths[0];
        lengths[0] = device.getName().length();

        // location
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = locationBuffer;
        bind[1].buffer_length = sizeof(locationBuffer);
        bind[1].length = &lengths[1];
        lengths[1] = device.getLocation().length();

        // status
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = statusBuffer;
        bind[2].buffer_length = sizeof(statusBuffer);
        bind[2].length = &lengths[2];
        lengths[2] = device.getStatus().length();

        // power
        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = powerBuffer;
        bind[3].buffer_length = sizeof(powerBuffer);
        bind[3].length = &lengths[3];
        lengths[3] = device.getPower().length();

        // brightness
        int brightness = device.getBrightness();
        bind[4].buffer_type = MYSQL_TYPE_LONG;
        bind[4].buffer = &brightness;

        // voltage
        double voltage = device.getVoltage();
        bind[5].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[5].buffer = &voltage;

        // current
        double current = device.getCurrent();
        bind[6].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[6].buffer = &current;

        // group_name
        bind[7].buffer_type = MYSQL_TYPE_STRING;
        bind[7].buffer = groupNameBuffer;
        bind[7].buffer_length = sizeof(groupNameBuffer);
        bind[7].length = &lengths[7];
        lengths[7] = device.getGroupName().length();

        // latitude
        double latitude = device.getLatitude();
        bind[8].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[8].buffer = &latitude;

        // longitude
        double longitude = device.getLongitude();
        bind[9].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[9].buffer = &longitude;

        // auto_mode
        bool auto_mode = device.getAutoMode();
        bind[10].buffer_type = MYSQL_TYPE_TINY;
        bind[10].buffer = &auto_mode;

        // voltage_threshold
        double voltage_threshold = device.getVoltageThreshold();
        bind[11].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[11].buffer = &voltage_threshold;

        // current_threshold
        double current_threshold = device.getCurrentThreshold();
        bind[12].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[12].buffer = &current_threshold;

        // on_time
        bind[13].buffer_type = MYSQL_TYPE_STRING;
        bind[13].buffer = onTimeBuffer;
        bind[13].buffer_length = sizeof(onTimeBuffer);
        bind[13].length = &lengths[13];
        lengths[13] = device.getOnTime().length();

        // off_time
        bind[14].buffer_type = MYSQL_TYPE_STRING;
        bind[14].buffer = offTimeBuffer;
        bind[14].buffer_length = sizeof(offTimeBuffer);
        bind[14].length = &lengths[14];
        lengths[14] = device.getOffTime().length();

        // uptime_hours
        int uptime_hours = device.getUptimeHours();
        bind[15].buffer_type = MYSQL_TYPE_LONG;
        bind[15].buffer = &uptime_hours;

        // last_maintenance (null)
        bind[16].buffer_type = MYSQL_TYPE_DATETIME;
        bind[16].is_null = static_cast<bool*>(malloc(sizeof(bool)));
        *bind[16].is_null = true;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            free(bind[16].is_null);
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定参数失败: " + error);
        }

        // 执行语句
        if (mysql_stmt_execute(stmt) != 0) {
            std::string error = mysql_stmt_error(stmt);
            free(bind[16].is_null);
            mysql_stmt_close(stmt);
            throw std::runtime_error("执行SQL语句失败: " + error);
        }

        // 检查影响的行数
        my_ulonglong affectedRows = mysql_stmt_affected_rows(stmt);
        free(bind[16].is_null);
        mysql_stmt_close(stmt);

        return affectedRows > 0;
    }
    catch(const std::exception& e)
    {
        spdlog::error("创建设备失败: {}", e.what());
        return false;
    }
}

std::shared_ptr<Device> DeviceRepository::getDeviceById(int id)
{
    MYSQL_STMT* stmt = nullptr;
    unsigned long* nameLength = nullptr;
    unsigned long* locationLength = nullptr;
    unsigned long* statusLength = nullptr;
    unsigned long* powerLength = nullptr;
    unsigned long* groupNameLength = nullptr;
    unsigned long* onTimeLength = nullptr;
    unsigned long* offTimeLength = nullptr;
    
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
            return nullptr;
        }

        // 准备SQL语句
        std::string sql = "SELECT id, name, location, status, power, brightness, voltage, current, group_name, latitude, longitude, auto_mode, voltage_threshold, current_threshold, on_time, off_time, uptime_hours, last_maintenance, created_at, updated_at FROM devices WHERE id = ?";
        stmt = mysql_stmt_init(conn);
        if (!stmt) {
            throw std::runtime_error("无法初始化SQL语句");
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("准备SQL语句失败: " + error);
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定参数失败: " + error);
        }

        // 执行查询
        if (mysql_stmt_execute(stmt) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("执行查询失败: " + error);
        }

        // 绑定结果
        MYSQL_BIND resultBind[20];
        memset(resultBind, 0, sizeof(resultBind));

        int idResult = 0;
        char name[256];
        char location[256];
        char status[256];
        char power[256];
        int brightness = 0;
        double voltage = 0.0;
        double current = 0.0;
        char group_name[256];
        double latitude = 0.0;
        double longitude = 0.0;
        bool auto_mode = false;
        double voltage_threshold = 0.0;
        double current_threshold = 0.0;
        char on_time[256];
        char off_time[256];
        int uptime_hours = 0;
        MYSQL_TIME last_maintenance = {0};
        MYSQL_TIME created_at = {0};
        MYSQL_TIME updated_at = {0};

        bool last_maintenance_is_null = false;

        resultBind[0].buffer_type = MYSQL_TYPE_LONG;
        resultBind[0].buffer = &idResult;

        resultBind[1].buffer_type = MYSQL_TYPE_STRING;
        resultBind[1].buffer = name;
        resultBind[1].buffer_length = sizeof(name);
        nameLength = new unsigned long;
        resultBind[1].length = nameLength;

        resultBind[2].buffer_type = MYSQL_TYPE_STRING;
        resultBind[2].buffer = location;
        resultBind[2].buffer_length = sizeof(location);
        locationLength = new unsigned long;
        resultBind[2].length = locationLength;

        resultBind[3].buffer_type = MYSQL_TYPE_STRING;
        resultBind[3].buffer = status;
        resultBind[3].buffer_length = sizeof(status);
        statusLength = new unsigned long;
        resultBind[3].length = statusLength;

        resultBind[4].buffer_type = MYSQL_TYPE_STRING;
        resultBind[4].buffer = power;
        resultBind[4].buffer_length = sizeof(power);
        powerLength = new unsigned long;
        resultBind[4].length = powerLength;

        resultBind[5].buffer_type = MYSQL_TYPE_LONG;
        resultBind[5].buffer = &brightness;

        resultBind[6].buffer_type = MYSQL_TYPE_DOUBLE;
        resultBind[6].buffer = &voltage;

        resultBind[7].buffer_type = MYSQL_TYPE_DOUBLE;
        resultBind[7].buffer = &current;

        resultBind[8].buffer_type = MYSQL_TYPE_STRING;
        resultBind[8].buffer = group_name;
        resultBind[8].buffer_length = sizeof(group_name);
        groupNameLength = new unsigned long;
        resultBind[8].length = groupNameLength;

        resultBind[9].buffer_type = MYSQL_TYPE_DOUBLE;
        resultBind[9].buffer = &latitude;

        resultBind[10].buffer_type = MYSQL_TYPE_DOUBLE;
        resultBind[10].buffer = &longitude;

        resultBind[11].buffer_type = MYSQL_TYPE_TINY;
        resultBind[11].buffer = &auto_mode;

        resultBind[12].buffer_type = MYSQL_TYPE_DOUBLE;
        resultBind[12].buffer = &voltage_threshold;

        resultBind[13].buffer_type = MYSQL_TYPE_DOUBLE;
        resultBind[13].buffer = &current_threshold;

        resultBind[14].buffer_type = MYSQL_TYPE_STRING;
        resultBind[14].buffer = on_time;
        resultBind[14].buffer_length = sizeof(on_time);
        onTimeLength = new unsigned long;
        resultBind[14].length = onTimeLength;

        resultBind[15].buffer_type = MYSQL_TYPE_STRING;
        resultBind[15].buffer = off_time;
        resultBind[15].buffer_length = sizeof(off_time);
        offTimeLength = new unsigned long;
        resultBind[15].length = offTimeLength;

        resultBind[16].buffer_type = MYSQL_TYPE_LONG;
        resultBind[16].buffer = &uptime_hours;

        resultBind[17].buffer_type = MYSQL_TYPE_DATETIME;
        resultBind[17].buffer = &last_maintenance;
        resultBind[17].is_null = &last_maintenance_is_null;

        resultBind[18].buffer_type = MYSQL_TYPE_DATETIME;
        resultBind[18].buffer = &created_at;

        resultBind[19].buffer_type = MYSQL_TYPE_DATETIME;
        resultBind[19].buffer = &updated_at;

        if (mysql_stmt_bind_result(stmt, resultBind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            throw std::runtime_error("绑定结果失败: " + error);
        }

        // 获取结果
        if (mysql_stmt_fetch(stmt) == 0) {
            // 创建设备对象
            auto device = std::make_shared<Device>();
            device->setId(idResult);
            device->setName(std::string(name, *nameLength));
            device->setLocation(std::string(location, *locationLength));
            device->setStatus(std::string(status, *statusLength));
            device->setPower(std::string(power, *powerLength));
            device->setBrightness(brightness);
            device->setVoltage(voltage);
            device->setCurrent(current);
            device->setGroupName(std::string(group_name, *groupNameLength));
            device->setLatitude(latitude);
            device->setLongitude(longitude);
            device->setAutoMode(auto_mode);
            device->setVoltageThreshold(voltage_threshold);
            device->setCurrentThreshold(current_threshold);
            device->setOnTime(std::string(on_time, *onTimeLength));
            device->setOffTime(std::string(off_time, *offTimeLength));
            device->setUptimeHours(uptime_hours);

            // 清理内存
            delete nameLength;
            delete locationLength;
            delete statusLength;
            delete powerLength;
            delete groupNameLength;
            delete onTimeLength;
            delete offTimeLength;
            mysql_stmt_close(stmt);

            return device;
        }

        // 清理内存
        delete nameLength;
        delete locationLength;
        delete statusLength;
        delete powerLength;
        delete groupNameLength;
        delete onTimeLength;
        delete offTimeLength;
        mysql_stmt_close(stmt);

        return nullptr;
    }
    catch(const std::exception& e)
    {
        spdlog::error("通过ID获取设备失败: {}", e.what());
        // 确保在异常情况下释放所有资源
        delete nameLength;
        delete locationLength;
        delete statusLength;
        delete powerLength;
        delete groupNameLength;
        delete onTimeLength;
        delete offTimeLength;
        if (stmt) {
            mysql_stmt_close(stmt);
        }
        return nullptr;
    }
}

std::vector<Device> DeviceRepository::getAllDevices()
{
    std::vector<Device> devices;
    MYSQL_RES* result = nullptr;
    
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
            return devices;
        }

        // 执行查询
        std::string sql = "SELECT id, name, location, status, power, brightness, voltage, current, group_name, latitude, longitude, auto_mode, voltage_threshold, current_threshold, on_time, off_time, uptime_hours, last_maintenance, created_at, updated_at FROM devices";
        if (mysql_query(conn, sql.c_str()) != 0) {
            spdlog::error("执行设备查询失败");
            // 连接已经出现错误，不需要再调用任何MySQL API函数，包括mysql_error()，避免段错误
            throw std::runtime_error("执行查询失败");
        }

        // 获取结果集
        result = mysql_store_result(conn);
        if (!result) {
            // 检查是否因为查询返回空结果集
            if (mysql_field_count(conn) == 0) {
                // 空结果集，不是错误
                spdlog::info("设备查询返回空结果集");
                return devices;
            } else {
                std::string error = mysql_error(conn);
                spdlog::error("获取设备结果集失败: {}", error);
                // 检查连接是否仍然有效
                if (mysql_ping(conn) != 0) {
                    spdlog::error("数据库连接已失效");
                }
                throw std::runtime_error("获取结果集失败: " + error);
            }
        }

        // 获取字段数量
        int num_fields = mysql_num_fields(result);
        if (num_fields == 0) {
            spdlog::info("设备结果集字段数为0");
            mysql_free_result(result);
            return devices;
        }

        // 遍历结果
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)) != nullptr) {
            Device device;
            // 确保row[0]不为空，否则跳过该记录
            if (row[0] == nullptr) {
                continue;
            }
            device.setId(std::stoi(row[0]));
            device.setName(row[1] ? row[1] : "");
            device.setLocation(row[2] ? row[2] : "");
            device.setStatus(row[3] ? row[3] : "");
            device.setPower(row[4] ? row[4] : "");
            device.setBrightness(row[5] ? std::stoi(row[5]) : 0);
            device.setVoltage(row[6] ? std::stod(row[6]) : 0.0);
            device.setCurrent(row[7] ? std::stod(row[7]) : 0.0);
            device.setGroupName(row[8] ? row[8] : "");
            device.setLatitude(row[9] ? std::stod(row[9]) : 0.0);
            device.setLongitude(row[10] ? std::stod(row[10]) : 0.0);
            device.setAutoMode(row[11] ? (std::stoi(row[11]) != 0) : false);
            device.setVoltageThreshold(row[12] ? std::stod(row[12]) : 0.0);
            device.setCurrentThreshold(row[13] ? std::stod(row[13]) : 0.0);
            device.setOnTime(row[14] ? row[14] : "");
            device.setOffTime(row[15] ? row[15] : "");
            device.setUptimeHours(row[16] ? std::stoi(row[16]) : 0);
            devices.push_back(device);
        }

        // 清理结果集
        mysql_free_result(result);
        spdlog::info("获取设备成功，共 {} 个设备", devices.size());
    }
    catch(const std::exception& e)
    {
        spdlog::error("获取所有设备失败: {}", e.what());
        // 确保结果集被释放，防止内存泄漏
        if (result != nullptr) {
            mysql_free_result(result);
        }
    }

    return devices;
}

bool DeviceRepository::updateDevice(const Device& device)
{
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
            return false;
        }

        // 准备SQL语句
        std::string sql = "UPDATE devices SET name = ?, location = ?, status = ?, power = ?, brightness = ?, voltage = ?, current = ?, group_name = ?, latitude = ?, longitude = ?, auto_mode = ?, voltage_threshold = ?, current_threshold = ?, on_time = ?, off_time = ?, uptime_hours = ?, updated_at = NOW() WHERE id = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            throw std::runtime_error("无法初始化SQL语句");
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("准备SQL语句失败: " + error);
        }

        // 绑定参数
        MYSQL_BIND bind[18];
        memset(bind, 0, sizeof(bind));
        
        // 存储字符串长度
        unsigned long lengths[18];
        
        // 为每个字符串创建足够大的缓冲区
        char nameBuffer[256];
        char locationBuffer[256];
        char statusBuffer[256];
        char powerBuffer[256];
        char groupNameBuffer[256];
        char onTimeBuffer[256];
        char offTimeBuffer[256];
        
        // 复制字符串到缓冲区
        strncpy(nameBuffer, device.getName().c_str(), sizeof(nameBuffer) - 1);
        strncpy(locationBuffer, device.getLocation().c_str(), sizeof(locationBuffer) - 1);
        strncpy(statusBuffer, device.getStatus().c_str(), sizeof(statusBuffer) - 1);
        strncpy(powerBuffer, device.getPower().c_str(), sizeof(powerBuffer) - 1);
        strncpy(groupNameBuffer, device.getGroupName().c_str(), sizeof(groupNameBuffer) - 1);
        strncpy(onTimeBuffer, device.getOnTime().c_str(), sizeof(onTimeBuffer) - 1);
        strncpy(offTimeBuffer, device.getOffTime().c_str(), sizeof(offTimeBuffer) - 1);
        
        // 确保字符串以null结尾
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        locationBuffer[sizeof(locationBuffer) - 1] = '\0';
        statusBuffer[sizeof(statusBuffer) - 1] = '\0';
        powerBuffer[sizeof(powerBuffer) - 1] = '\0';
        groupNameBuffer[sizeof(groupNameBuffer) - 1] = '\0';
        onTimeBuffer[sizeof(onTimeBuffer) - 1] = '\0';
        offTimeBuffer[sizeof(offTimeBuffer) - 1] = '\0';

        // name
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = nameBuffer;
        bind[0].buffer_length = sizeof(nameBuffer);
        bind[0].length = &lengths[0];
        lengths[0] = device.getName().length();

        // location
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = locationBuffer;
        bind[1].buffer_length = sizeof(locationBuffer);
        bind[1].length = &lengths[1];
        lengths[1] = device.getLocation().length();

        // status
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = statusBuffer;
        bind[2].buffer_length = sizeof(statusBuffer);
        bind[2].length = &lengths[2];
        lengths[2] = device.getStatus().length();

        // power
        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = powerBuffer;
        bind[3].buffer_length = sizeof(powerBuffer);
        bind[3].length = &lengths[3];
        lengths[3] = device.getPower().length();

        // brightness
        int brightness = device.getBrightness();
        bind[4].buffer_type = MYSQL_TYPE_LONG;
        bind[4].buffer = &brightness;

        // voltage
        double voltage = device.getVoltage();
        bind[5].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[5].buffer = &voltage;

        // current
        double current = device.getCurrent();
        bind[6].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[6].buffer = &current;

        // group_name
        bind[7].buffer_type = MYSQL_TYPE_STRING;
        bind[7].buffer = groupNameBuffer;
        bind[7].buffer_length = sizeof(groupNameBuffer);
        bind[7].length = &lengths[7];
        lengths[7] = device.getGroupName().length();

        // latitude
        double latitude = device.getLatitude();
        bind[8].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[8].buffer = &latitude;

        // longitude
        double longitude = device.getLongitude();
        bind[9].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[9].buffer = &longitude;

        // auto_mode
        bool auto_mode = device.getAutoMode();
        bind[10].buffer_type = MYSQL_TYPE_TINY;
        bind[10].buffer = &auto_mode;

        // voltage_threshold
        double voltage_threshold = device.getVoltageThreshold();
        bind[11].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[11].buffer = &voltage_threshold;

        // current_threshold
        double current_threshold = device.getCurrentThreshold();
        bind[12].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[12].buffer = &current_threshold;

        // on_time
        bind[13].buffer_type = MYSQL_TYPE_STRING;
        bind[13].buffer = onTimeBuffer;
        bind[13].buffer_length = sizeof(onTimeBuffer);
        bind[13].length = &lengths[13];
        lengths[13] = device.getOnTime().length();

        // off_time
        bind[14].buffer_type = MYSQL_TYPE_STRING;
        bind[14].buffer = offTimeBuffer;
        bind[14].buffer_length = sizeof(offTimeBuffer);
        bind[14].length = &lengths[14];
        lengths[14] = device.getOffTime().length();

        // uptime_hours
        int uptime_hours = device.getUptimeHours();
        bind[15].buffer_type = MYSQL_TYPE_LONG;
        bind[15].buffer = &uptime_hours;

        // id
        int id = device.getId();
        bind[16].buffer_type = MYSQL_TYPE_LONG;
        bind[16].buffer = &id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定参数失败: " + error);
        }

        // 执行语句
        if (mysql_stmt_execute(stmt) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("执行SQL语句失败: " + error);
        }

        // 检查影响的行数
        my_ulonglong affectedRows = mysql_stmt_affected_rows(stmt);
        mysql_stmt_close(stmt);

        return affectedRows > 0;
    }
    catch(const std::exception& e)
    {
        spdlog::error("更新设备失败: {}", e.what());
        return false;
    }
}

bool DeviceRepository::deleteDevice(int id)
{
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
            return false;
        }

        // 准备SQL语句
        std::string sql = "DELETE FROM devices WHERE id = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            throw std::runtime_error("无法初始化SQL语句");
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("准备SQL语句失败: " + error);
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定参数失败: " + error);
        }

        // 执行语句
        if (mysql_stmt_execute(stmt) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("执行SQL语句失败: " + error);
        }

        // 检查影响的行数
        my_ulonglong affectedRows = mysql_stmt_affected_rows(stmt);
        mysql_stmt_close(stmt);

        return affectedRows > 0;
    }
    catch(const std::exception& e)
    {
        spdlog::error("删除设备失败: {}", e.what());
        return false;
    }
}

bool DeviceRepository::controlDevice(int id, const std::string& action, int brightness)
{
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
            return false;
        }

        // 获取设备信息
        std::string deviceName;
        std::string groupName;
        std::string getDeviceSql = "SELECT name, group_name FROM devices WHERE id = ?";
        MYSQL_STMT* getDeviceStmt = mysql_stmt_init(conn);
        if (!getDeviceStmt) {
            throw std::runtime_error("无法初始化获取设备信息的SQL语句");
        }
        
        MYSQL_BIND getBind[1];
        memset(getBind, 0, sizeof(getBind));
        getBind[0].buffer_type = MYSQL_TYPE_LONG;
        getBind[0].buffer = &id;
        
        if (mysql_stmt_prepare(getDeviceStmt, getDeviceSql.c_str(), getDeviceSql.length()) != 0) {
            std::string error = mysql_stmt_error(getDeviceStmt);
            mysql_stmt_close(getDeviceStmt);
            throw std::runtime_error("准备获取设备信息的SQL语句失败: " + error);
        }
        
        if (mysql_stmt_bind_param(getDeviceStmt, getBind) != 0) {
            std::string error = mysql_stmt_error(getDeviceStmt);
            mysql_stmt_close(getDeviceStmt);
            throw std::runtime_error("绑定获取设备信息的参数失败: " + error);
        }
        
        if (mysql_stmt_execute(getDeviceStmt) != 0) {
            std::string error = mysql_stmt_error(getDeviceStmt);
            mysql_stmt_close(getDeviceStmt);
            throw std::runtime_error("执行获取设备信息的SQL语句失败: " + error);
        }
        
        // 绑定结果
        MYSQL_BIND resultBind[2];
        memset(resultBind, 0, sizeof(resultBind));
        
        char nameBuffer[256];
        char groupBuffer[256];
        unsigned long nameLength = 0;
        unsigned long groupLength = 0;
        
        resultBind[0].buffer_type = MYSQL_TYPE_STRING;
        resultBind[0].buffer = nameBuffer;
        resultBind[0].buffer_length = sizeof(nameBuffer);
        resultBind[0].length = &nameLength;
        
        resultBind[1].buffer_type = MYSQL_TYPE_STRING;
        resultBind[1].buffer = groupBuffer;
        resultBind[1].buffer_length = sizeof(groupBuffer);
        resultBind[1].length = &groupLength;
        
        if (mysql_stmt_bind_result(getDeviceStmt, resultBind) != 0) {
            std::string error = mysql_stmt_error(getDeviceStmt);
            mysql_stmt_close(getDeviceStmt);
            throw std::runtime_error("绑定获取设备信息的结果失败: " + error);
        }
        
        // 获取结果
        if (mysql_stmt_fetch(getDeviceStmt) != 0) {
            mysql_stmt_close(getDeviceStmt);
            throw std::runtime_error("获取设备信息失败: 设备不存在");
        }
        
        // 处理结果
        nameBuffer[nameLength] = '\0';
        groupBuffer[groupLength] = '\0';
        deviceName = std::string(nameBuffer);
        groupName = std::string(groupBuffer);
        
        mysql_stmt_close(getDeviceStmt);
        
        // 准备SQL语句
        std::string sql = "UPDATE devices SET power = ?, brightness = ?, updated_at = NOW() WHERE id = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            throw std::runtime_error("无法初始化SQL语句");
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("准备SQL语句失败: " + error);
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));
        
        // 存储字符串长度
        unsigned long lengths[3];
        
        // 为每个字符串创建足够大的缓冲区
        char powerBuffer[256];

        std::string power;
        
        // 复制字符串到缓冲区
        if (strcmp(action.c_str(), "turn_on") == 0)
        {   
            power = "ON";
            strncpy(powerBuffer, power.c_str(), sizeof(powerBuffer) - 1);
            spdlog::info("发布MQTT消息，控制小灯开");
            LampMonitor::getInstance().controlLamp("switch", power, groupName[0], deviceName, brightness);
        }
        else if (strcmp(action.c_str(), "turn_off") == 0)
        {
            power = "OFF";
            strncpy(powerBuffer, power.c_str(), sizeof(powerBuffer) - 1);
            spdlog::info("发布MQTT消息，控制小灯关");
            LampMonitor::getInstance().controlLamp("switch", power, groupName[0], deviceName, brightness);
        }
        else if (strcmp(action.c_str(), "set_brightness") == 0)
        {
            if (brightness == 0)
            {
                power = "OFF";
                strncpy(powerBuffer, power.c_str(), sizeof(powerBuffer) - 1);
            }
            else
            {
                power = "ON";
                strncpy(powerBuffer, power.c_str(), sizeof(powerBuffer) - 1);
            }
            spdlog::info("发布MQTT消息，控制小灯设置亮度");
            LampMonitor::getInstance().controlLamp("dim", power, groupName[0], deviceName, brightness);
        }
        powerBuffer[sizeof(powerBuffer) - 1] = '\0';

        // power
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = powerBuffer;
        bind[0].buffer_length = sizeof(powerBuffer);
        bind[0].length = &lengths[0];
        lengths[0] = power.length();

        // brightness
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &brightness;

        // id
        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定参数失败: " + error);
        }

        // 执行语句
        if (mysql_stmt_execute(stmt) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("执行SQL语句失败: " + error);
        }

        // 检查影响的行数
        my_ulonglong affectedRows = mysql_stmt_affected_rows(stmt);
        mysql_stmt_close(stmt);

        return affectedRows > 0;
    }
    catch(const std::exception& e)
    {
        spdlog::error("控制设备失败: {}", e.what());
        return false;
    }
}

bool DeviceRepository::controlGroup(const std::string& group_name, const std::string& power, int brightness)
{
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
            return false;
        }

        // 准备SQL语句
        std::string sql = "UPDATE devices SET power = ?, brightness = ?, updated_at = NOW() WHERE group_name = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            throw std::runtime_error("无法初始化SQL语句");
        }

        if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length()) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("准备SQL语句失败: " + error);
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));
        
        // 存储字符串长度
        unsigned long lengths[3];
        
        // 为每个字符串创建足够大的缓冲区
        char powerBuffer[256];
        char groupNameBuffer[256];
        
        // 复制字符串到缓冲区
        strncpy(powerBuffer, power.c_str(), sizeof(powerBuffer) - 1);
        strncpy(groupNameBuffer, group_name.c_str(), sizeof(groupNameBuffer) - 1);
        
        // 确保字符串以null结尾
        powerBuffer[sizeof(powerBuffer) - 1] = '\0';
        groupNameBuffer[sizeof(groupNameBuffer) - 1] = '\0';

        // power
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = powerBuffer;
        bind[0].buffer_length = sizeof(powerBuffer);
        bind[0].length = &lengths[0];
        lengths[0] = power.length();

        // brightness
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &brightness;

        // group_name
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = groupNameBuffer;
        bind[2].buffer_length = sizeof(groupNameBuffer);
        bind[2].length = &lengths[2];
        lengths[2] = group_name.length();

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("绑定参数失败: " + error);
        }

        // 执行语句
        if (mysql_stmt_execute(stmt) != 0) {
            std::string error = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("执行SQL语句失败: " + error);
        }

        // 检查影响的行数
        my_ulonglong affectedRows = mysql_stmt_affected_rows(stmt);
        mysql_stmt_close(stmt);

        return affectedRows > 0;
    }
    catch(const std::exception& e)
    {
        spdlog::error("控制设备组失败: {}", e.what());
        return false;
    }
}

bool DeviceRepository::batchControl(const std::vector<int>& device_ids, const std::string& power, int brightness)
{
    try
    {
        MYSQL* conn = dbConnection.getConnection();
        if (conn == nullptr) {
            spdlog::error("无法获取数据库连接");
            return false;
        }

        // 构建IN子句
        std::string in_clause = "(";
        for(size_t i=0; i<device_ids.size(); i++) {
            if(i > 0) in_clause += ",";
            in_clause += std::to_string(device_ids[i]);
        }
        in_clause += ")";

        // 准备SQL语句
        std::string sql = "UPDATE devices SET power = ?, brightness = ?, updated_at = NOW() WHERE id IN " + in_clause;
        
        // 执行查询
        if (mysql_query(conn, sql.c_str()) != 0) {
            throw std::runtime_error("执行查询失败: " + std::string(mysql_error(conn)));
        }

        // 检查影响的行数
        my_ulonglong affectedRows = mysql_affected_rows(conn);
        return affectedRows > 0;
    }
    catch(const std::exception& e)
    {
        spdlog::error("批量控制设备失败: {}", e.what());
        return false;
    }
}
