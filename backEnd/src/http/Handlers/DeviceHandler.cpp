#include "DeviceHandler.h"
#include "Utils/JsonParser.h"
#include "Utils/Constants.h"
#include "Middleware/AuthMiddleware.h"
#include <spdlog/spdlog.h>

DeviceHandler::DeviceHandler() : deviceRepo(std::make_unique<DeviceRepository>()) {}

DeviceHandler::~DeviceHandler() {}

void DeviceHandler::registerRoutes(httplib::Server& svr)
{
    // 获取所有设备
    svr.Get("/api/devices", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetDevices(req, res);
    });

    // 获取单个设备详情
    svr.Get(R"(/api/devices/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetDevice(req, res);
    });
    
    // 创建设备
    svr.Post("/api/devices", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleCreateDevice(req, res);
    });
    
    // 更新设备
    svr.Put(R"(/api/devices/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleUpdateDevice(req, res);
    });
    
    // 删除设备
    svr.Delete(R"(/api/devices/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleDeleteDevice(req, res);
    });
    
    // 控制设备
    svr.Post(R"(/api/devices/(\d+)/control)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleControlDevice(req, res);
    });
    
    // 分组控制设备
    svr.Post("/api/devices/group-control", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleControlGroup(req, res);
    });
    
    // 批量控制设备
    svr.Post("/api/devices/batch-control", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleBatchControl(req, res);
    });
}

void DeviceHandler::handleGetDevices(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // 验证令牌
        if(!AuthMiddleware::validateToken(req))
        {
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            res.set_content(R"({"error": "未授权访问"})", "application/json");
            return;
        }

        // 获取所有设备
        auto devices = deviceRepo->getAllDevices();
        
        // 构建响应
        nlohmann::json response;
        response["success"] = true;
        response["devices"] = nlohmann::json::array();
        
        for(const auto& device : devices)
        {
            nlohmann::json deviceJson;
            deviceJson["id"] = device.getId();
            deviceJson["name"] = device.getName();
            deviceJson["location"] = device.getLocation();
            deviceJson["status"] = device.getStatus();
            deviceJson["power"] = device.getPower();
            deviceJson["brightness"] = device.getBrightness();
            deviceJson["voltage"] = device.getVoltage();
            deviceJson["current"] = device.getCurrent();
            deviceJson["groupName"] = device.getGroupName();
            deviceJson["latitude"] = device.getLatitude();
            deviceJson["longitude"] = device.getLongitude();
            deviceJson["autoMode"] = device.getAutoMode();
            deviceJson["voltageThreshold"] = device.getVoltageThreshold();
            deviceJson["currentThreshold"] = device.getCurrentThreshold();
            deviceJson["onTime"] = device.getOnTime();
            deviceJson["offTime"] = device.getOffTime();
            deviceJson["uptimeHours"] = device.getUptimeHours();
            response["devices"].push_back(deviceJson);
        }
        
        res.status = Constants::RESPONSE_SUCCESS;
        res.set_content(JsonParser::serialize(response), "application/json");
    }
    catch(const std::exception& e)
    {
        spdlog::error("获取设备列表失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void DeviceHandler::handleGetDevice(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // 验证令牌
        if(!AuthMiddleware::validateToken(req))
        {
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            res.set_content(R"({"error": "未授权访问"})", "application/json");
            return;
        }

        // 获取设备ID
        int id = std::stoi(req.matches[1]);
        
        // 获取设备详情
        auto device = deviceRepo->getDeviceById(id);
        
        if(!device)
        {
            res.status = Constants::RESPONSE_NOT_FOUND;
            res.set_content(R"({"error": "设备不存在"})", "application/json");
            return;
        }
        
        // 构建响应
        nlohmann::json response;
        response["success"] = true;
        response["device"] = {
            {"id", device->getId()},
            {"name", device->getName()},
            {"location", device->getLocation()},
            {"status", device->getStatus()},
            {"power", device->getPower()},
            {"brightness", device->getBrightness()},
            {"voltage", device->getVoltage()},
            {"current", device->getCurrent()},
            {"groupName", device->getGroupName()},
            {"latitude", device->getLatitude()},
            {"longitude", device->getLongitude()},
            {"autoMode", device->getAutoMode()},
            {"voltageThreshold", device->getVoltageThreshold()},
            {"currentThreshold", device->getCurrentThreshold()},
            {"onTime", device->getOnTime()},
            {"offTime", device->getOffTime()},
            {"uptimeHours", device->getUptimeHours()}
        };
        
        res.status = Constants::RESPONSE_SUCCESS;
        res.set_content(JsonParser::serialize(response), "application/json");
    }
    catch(const std::exception& e)
    {
        spdlog::error("获取设备详情失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void DeviceHandler::handleCreateDevice(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // 验证令牌
        if(!AuthMiddleware::validateToken(req))
        {
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            res.set_content(R"({"error": "未授权访问"})", "application/json");
            return;
        }

        // 解析请求体
        auto jsonData = JsonParser::parse(req.body);
        
        // 验证必填字段
        if(!jsonData.contains("name") || !jsonData.contains("location") || !jsonData.contains("groupName"))
        {
            res.status = Constants::RESPONSE_BAD_REQUEST;
            res.set_content(R"({"error": "缺少必填字段"})", "application/json");
            return;
        }
        
        // 创建设备对象
        Device device;
        device.setName(jsonData["name"]);
        device.setLocation(jsonData["location"]);
        device.setGroupName(jsonData["groupName"]);
        
        // 可选字段
        if(jsonData.contains("status")) device.setStatus(jsonData["status"]);
        if(jsonData.contains("power")) device.setPower(jsonData["power"]);
        if(jsonData.contains("brightness")) device.setBrightness(jsonData["brightness"]);
        if(jsonData.contains("voltage")) device.setVoltage(jsonData["voltage"]);
        if(jsonData.contains("current")) device.setCurrent(jsonData["current"]);
        if(jsonData.contains("latitude")) device.setLatitude(jsonData["latitude"]);
        if(jsonData.contains("longitude")) device.setLongitude(jsonData["longitude"]);
        if(jsonData.contains("autoMode")) device.setAutoMode(jsonData["autoMode"]);
        if(jsonData.contains("voltageThreshold")) device.setVoltageThreshold(jsonData["voltageThreshold"]);
        if(jsonData.contains("currentThreshold")) device.setCurrentThreshold(jsonData["currentThreshold"]);
        if(jsonData.contains("onTime")) device.setOnTime(jsonData["onTime"]);
        if(jsonData.contains("offTime")) device.setOffTime(jsonData["offTime"]);
        if(jsonData.contains("uptimeHours")) device.setUptimeHours(jsonData["uptimeHours"]);
        
        // 保存设备
        if(deviceRepo->createDevice(device))
        {
            res.status = Constants::RESPONSE_SUCCESS;
            res.set_content(R"({"success": true, "message": "设备创建成功"})", "application/json");
        }
        else
        {
            res.status = Constants::RESPONSE_INTERNAL_ERROR;
            res.set_content(R"({"error": "设备创建失败"})", "application/json");
        }
    }
    catch(const std::exception& e)
    {
        spdlog::error("创建设备失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void DeviceHandler::handleUpdateDevice(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // 验证令牌
        if(!AuthMiddleware::validateToken(req))
        {
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            res.set_content(R"({"error": "未授权访问"})", "application/json");
            return;
        }

        spdlog::info("请求体: {}", req.body);
        // 获取设备ID
        int id = std::stoi(req.matches[1]);
        
        // 检查设备是否存在
        auto existingDevice = deviceRepo->getDeviceById(id);
        if(!existingDevice)
        {
            res.status = Constants::RESPONSE_NOT_FOUND;
            res.set_content(R"({"error": "设备不存在"})", "application/json");
            return;
        }
        
        // 解析请求体
        auto jsonData = JsonParser::parse(req.body);
        
        // 更新设备字段
        Device device = *existingDevice;
        if(jsonData.contains("name")) device.setName(jsonData["name"]);
        if(jsonData.contains("location")) device.setLocation(jsonData["location"]);
        if(jsonData.contains("status")) device.setStatus(jsonData["status"]);
        if(jsonData.contains("power")) device.setPower(jsonData["power"]);
        if(jsonData.contains("brightness")) device.setBrightness(jsonData["brightness"]);
        if(jsonData.contains("voltage")) device.setVoltage(jsonData["voltage"]);
        if(jsonData.contains("current")) device.setCurrent(jsonData["current"]);
        if(jsonData.contains("groupName")) device.setGroupName(jsonData["groupName"]);
        if(jsonData.contains("latitude")) device.setLatitude(jsonData["latitude"]);
        if(jsonData.contains("longitude")) device.setLongitude(jsonData["longitude"]);
        if(jsonData.contains("autoMode")) device.setAutoMode(jsonData["autoMode"]);
        if(jsonData.contains("voltageThreshold")) device.setVoltageThreshold(jsonData["voltageThreshold"]);
        if(jsonData.contains("currentThreshold")) device.setCurrentThreshold(jsonData["currentThreshold"]);
        if(jsonData.contains("uptimeHours")) device.setUptimeHours(jsonData["uptimeHours"]);
        if (jsonData.contains("schedule"))
        {
            auto scheduleData = jsonData["schedule"];
            device.setOnTime(scheduleData["onTime"]);
            device.setOffTime(scheduleData["offTime"]);
        }
        
        // 保存更新
        if(deviceRepo->updateDevice(device))
        {
            res.status = Constants::RESPONSE_SUCCESS;

            // 构造JSON响应数据
            nlohmann::json response;
            response["group"] = device.getGroupName();  // 设备分组
            response["message"] = "设备更新成功";  // 操作成功消息
            response["power"] = device.getPower();  // 当前电源状态
            response["schedule"] = {
                {"onTime", device.getOnTime()},
                {"offTime", device.getOffTime()}
            };
            response["status"] = device.getStatus();  // 设备状态
            response["success"] = true;  // 操作是否成功
            response["uptime"] = device.getUptimeHours();  // 运行时间

            res.set_content(JsonParser::serialize(response), "application/json");
        }
        else
        {
            res.status = Constants::RESPONSE_INTERNAL_ERROR;
            res.set_content(R"({"error": "设备更新失败"})", "application/json");
        }
    }
    catch(const std::exception& e)
    {
        spdlog::error("更新设备失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void DeviceHandler::handleDeleteDevice(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // 验证令牌
        if(!AuthMiddleware::validateToken(req))
        {
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            res.set_content(R"({"error": "未授权访问"})", "application/json");
            return;
        }

        // 获取设备ID
        int id = std::stoi(req.matches[1]);
        
        // 删除设备
        if(deviceRepo->deleteDevice(id))
        {
            res.status = Constants::RESPONSE_SUCCESS;
            res.set_content(R"({"success": true, "message": "设备删除成功"})", "application/json");
        }
        else
        {
            res.status = Constants::RESPONSE_INTERNAL_ERROR;
            res.set_content(R"({"error": "设备删除失败"})", "application/json");
        }
    }
    catch(const std::exception& e)
    {
        spdlog::error("删除设备失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void DeviceHandler::handleControlDevice(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // 验证令牌
        if(!AuthMiddleware::validateToken(req))
        {
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            res.set_content(R"({"error": "未授权访问"})", "application/json");
            return;
        }

        // 获取设备ID
        int id = std::stoi(req.matches[1]);
        
        // 解析请求体
        auto jsonData = JsonParser::parse(req.body);

        spdlog::info("控制设备: {}", id);
        spdlog::info("控制参数: {}", jsonData.dump());
        
        // 验证必填字段
        if(!jsonData.contains("action"))
        {
            res.status = Constants::RESPONSE_BAD_REQUEST;
            res.set_content(R"({"error": "缺少必填字段"})", "application/json");
            return;
        }
        
        // 获取控制参数
        std::string action = jsonData["action"];
        int brightness = jsonData.contains("brightness") ? jsonData["brightness"].get<int>() : 0;
        
        // 控制设备
        if(deviceRepo->controlDevice(id, action, brightness))
        {
            res.status = Constants::RESPONSE_SUCCESS;
            res.set_content(R"({"success": true, "message": "设备控制成功"})", "application/json");
        }
        else
        {
            res.status = Constants::RESPONSE_INTERNAL_ERROR;
            res.set_content(R"({"error": "设备控制失败"})", "application/json");
        }
    }
    catch(const std::exception& e)
    {
        spdlog::error("控制设备失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void DeviceHandler::handleControlGroup(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // 验证令牌
        if(!AuthMiddleware::validateToken(req))
        {
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            res.set_content(R"({"error": "未授权访问"})", "application/json");
            return;
        }

        // 解析请求体
        auto jsonData = JsonParser::parse(req.body);
        spdlog::info("控制设备组: {}", jsonData.dump());
        // {"action":"set_brightness","brightness":"55","group":"A区"}
        
        // 验证必填字段
        if(!jsonData.contains("group") || !jsonData.contains("brightness"))
        {
            res.status = Constants::RESPONSE_BAD_REQUEST;
            res.set_content(R"({"error": "缺少必填字段"})", "application/json");
            return;
        }
        
        // 获取控制参数
        std::string group = jsonData["group"];
        int brightness = 0;
        if(jsonData.contains("brightness"))
        {
            try
            {
                // 尝试将brightness转换为整数
                if(jsonData["brightness"].is_number())
                {
                    brightness = jsonData["brightness"].get<int>();
                }
                else if(jsonData["brightness"].is_string())
                {
                    // 如果是字符串，尝试转换为整数
                    brightness = std::stoi(jsonData["brightness"].get<std::string>());
                }
            }
            catch(const std::exception& e)
            {
                spdlog::error("亮度值转换失败: {}", e.what());
                res.status = Constants::RESPONSE_BAD_REQUEST;
                res.set_content(R"({"error": "亮度值格式不正确"})", "application/json");
                return;
            }
        }
        // 正确初始化power字符串
        std::string power = (brightness > 0) ? "ON" : "OFF";

        std::string action = jsonData["action"];

        // 控制设备组
        if(deviceRepo->controlGroup(action, group, power, brightness))
        {
            res.status = Constants::RESPONSE_SUCCESS;
            res.set_content(R"({"success": true, "message": "设备组控制成功"})", "application/json");
        }
        else
        {
            res.status = Constants::RESPONSE_INTERNAL_ERROR;
            res.set_content(R"({"error": "设备组控制失败"})", "application/json");
        }
    }
    catch(const std::exception& e)
    {
        spdlog::error("控制设备组失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void DeviceHandler::handleBatchControl(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // 验证令牌
        if(!AuthMiddleware::validateToken(req))
        {
            res.status = Constants::RESPONSE_UNAUTHORIZED;
            res.set_content(R"({"error": "未授权访问"})", "application/json");
            return;
        }

        // 解析请求体
        auto jsonData = JsonParser::parse(req.body);
        
        // 验证必填字段
        if(!jsonData.contains("deviceIds") || !jsonData.contains("power"))
        {
            res.status = Constants::RESPONSE_BAD_REQUEST;
            res.set_content(R"({"error": "缺少必填字段"})", "application/json");
            return;
        }
        
        // 获取控制参数
        auto deviceIdsArray = jsonData["deviceIds"];
        std::vector<int> deviceIds;
        for(auto& id : deviceIdsArray)
        {
            deviceIds.push_back(id);
        }
        
        std::string power = jsonData["power"];
        int brightness = jsonData.contains("brightness") ? jsonData["brightness"].get<int>() : 0;
        
        // 批量控制设备
        if(deviceRepo->batchControl(deviceIds, power, brightness))
        {
            res.status = Constants::RESPONSE_SUCCESS;
            res.set_content(R"({"success": true, "message": "批量控制成功"})", "application/json");
        }
        else
        {
            res.status = Constants::RESPONSE_INTERNAL_ERROR;
            res.set_content(R"({"error": "批量控制失败"})", "application/json");
        }
    }
    catch(const std::exception& e)
    {
        spdlog::error("批量控制设备失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}
