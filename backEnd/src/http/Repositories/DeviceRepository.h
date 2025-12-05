#pragma once

#include <string>
#include <memory>
#include <vector>
#include "Models/Device.h"
#include "Utils/DatabaseConnection.h"

class DeviceRepository
{
public:
    DeviceRepository();
    ~DeviceRepository();
    
    // 创建设备
    bool createDevice(const Device& device);
    
    // 通过ID查找设备
    std::shared_ptr<Device> getDeviceById(int id);
    
    // 获取所有设备
    std::vector<Device> getAllDevices();
    
    // 更新设备信息
    bool updateDevice(const Device& device);
    
    // 删除设备
    bool deleteDevice(int id);

    // 控制设备
    bool controlDevice(int id, const std::string& power, int brightness);

    // 分组控制设备
    bool controlGroup(const std::string& group_name, const std::string& power, int brightness);

    // 批量控制设备
    bool batchControl(const std::vector<int>& device_ids, const std::string& power, int brightness);

private:
    // 数据库连接
    DatabaseConnection dbConnection;
};