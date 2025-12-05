#pragma once

#include <string>
#include <memory>
#include <vector>
#include <httplib.h>
#include "Repositories/DeviceRepository.h"

class DeviceHandler
{
public:
    DeviceHandler();
    ~DeviceHandler();

    // 注册路由
    void registerRoutes(httplib::Server& svr);

private:
    // 处理获取所有设备
    void handleGetDevices(const httplib::Request& req, httplib::Response& res);
    
    // 处理获取单个设备详情
    void handleGetDevice(const httplib::Request& req, httplib::Response& res);
    
    // 处理创建设备
    void handleCreateDevice(const httplib::Request& req, httplib::Response& res);
    
    // 处理更新设备
    void handleUpdateDevice(const httplib::Request& req, httplib::Response& res);
    
    // 处理删除设备
    void handleDeleteDevice(const httplib::Request& req, httplib::Response& res);
    
    // 处理控制设备
    void handleControlDevice(const httplib::Request& req, httplib::Response& res);
    
    // 处理分组控制设备
    void handleControlGroup(const httplib::Request& req, httplib::Response& res);
    
    // 处理批量控制设备
    void handleBatchControl(const httplib::Request& req, httplib::Response& res);

    // 设备仓库实例
    std::unique_ptr<DeviceRepository> deviceRepo;
};