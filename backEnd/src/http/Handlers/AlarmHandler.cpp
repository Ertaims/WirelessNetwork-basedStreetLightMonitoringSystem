#include "AlarmHandler.h"
#include "Utils/JsonParser.h"
#include "Utils/Constants.h"
#include "Middleware/AuthMiddleware.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

AlarmHandler::AlarmHandler() : alarmRepo(std::make_unique<AlarmRepository>()) {}

AlarmHandler::~AlarmHandler() {}

void AlarmHandler::registerRoutes(httplib::Server& svr)
{
    // 获取告警列表
    svr.Get("/api/alarms", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetAlarms(req, res);
    });

    // 获取告警统计
    svr.Get("/api/alarms/stats", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleGetAlarmStats(req, res);
    });
    
    // 确认告警
    svr.Put(R"(/api/alarms/(\d+)/confirm)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleConfirmAlarm(req, res);
    });
    
    // 解决告警
    svr.Put(R"(/api/alarms/(\d+)/resolve)", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleResolveAlarm(req, res);
    });
    
    // 批量处理告警
    svr.Post("/api/alarms/batch-process", [this](const httplib::Request& req, httplib::Response& res) {
        this->handleBatchProcessAlarms(req, res);
    });
}

void AlarmHandler::handleGetAlarms(const httplib::Request& req, httplib::Response& res)
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

        // 获取查询参数
        int page = 1;
        int pageSize = 20;
        
        // 安全地转换page参数
        if (req.has_param("page")) {
            try {
                page = std::stoi(req.get_param_value("page"));
                if (page < 1) page = 1;
            } catch (const std::exception& e) {
                spdlog::warn("无效的page参数，使用默认值1");
                page = 1;
            }
        }
        
        // 安全地转换pageSize参数
        if (req.has_param("pageSize")) {
            try {
                pageSize = std::stoi(req.get_param_value("pageSize"));
                if (pageSize < 1 || pageSize > 100) pageSize = 20;
            } catch (const std::exception& e) {
                spdlog::warn("无效的pageSize参数，使用默认值20");
                pageSize = 20;
            }
        }
        
        std::string status = req.has_param("status") ? req.get_param_value("status") : "";
        std::string level = req.has_param("level") ? req.get_param_value("level") : "";

        // 获取告警列表
        auto alarms = alarmRepo->getAlarms(page, pageSize, status, level);
        
        // 构建响应
        nlohmann::json response;
        response["success"] = true;
        response["alarms"] = nlohmann::json::array();
        
        for(const auto& alarm : alarms)
        {
            nlohmann::json alarmJson;
            alarmJson["id"] = alarm.getId();
            alarmJson["deviceId"] = alarm.getDeviceId();
            alarmJson["type"] = alarm.getType();
            alarmJson["level"] = alarm.getLevel();
            alarmJson["status"] = alarm.getStatus();
            alarmJson["description"] = alarm.getDescription();
            alarmJson["value"] = alarm.getValue();
            alarmJson["threshold"] = alarm.getThreshold();
            alarmJson["occurredAt"] = alarm.getOccurredAt();
            alarmJson["confirmedAt"] = alarm.getConfirmedAt();
            alarmJson["resolvedAt"] = alarm.getResolvedAt();
            alarmJson["remark"] = alarm.getRemark();
            alarmJson["confirmedBy"] = alarm.getConfirmedBy();
            alarmJson["resolvedBy"] = alarm.getResolvedBy();
            
            response["alarms"].push_back(alarmJson);
        }
        
        res.status = Constants::RESPONSE_SUCCESS;
        res.set_content(JsonParser::serialize(response), "application/json");
    }
    catch(const std::exception& e)
    {
        spdlog::error("获取告警列表失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void AlarmHandler::handleGetAlarmStats(const httplib::Request& req, httplib::Response& res)
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

        // 获取告警统计
        auto stats = alarmRepo->getAlarmStats();
        
        // 构建响应
        nlohmann::json response;
        response["success"] = true;
        response["stats"] = nlohmann::json::object();
        
        for(const auto& stat : stats)
        {
            response["stats"][stat.first] = stat.second;
        }
        
        res.status = Constants::RESPONSE_SUCCESS;
        res.set_content(JsonParser::serialize(response), "application/json");
    }
    catch(const std::exception& e)
    {
        spdlog::error("获取告警统计失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void AlarmHandler::handleConfirmAlarm(const httplib::Request& req, httplib::Response& res)
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

        // 获取告警ID
        int alarmId = std::stoi(req.matches[1]);
        
        // 解析请求体
        auto requestBody = JsonParser::parse(req.body);
        std::string remark = requestBody.value("remark", "");
        std::string confirmedBy = requestBody.value("confirmedBy", "admin");
        
        // 确认告警
        bool success = alarmRepo->confirmAlarm(alarmId, remark, confirmedBy);
        
        // 构建响应
        nlohmann::json response;
        if(success)
        {
            response["success"] = true;
            response["message"] = "告警确认成功";
            res.status = Constants::RESPONSE_SUCCESS;
        }
        else
        {
            response["success"] = false;
            response["message"] = "告警确认失败";
            res.status = Constants::RESPONSE_BAD_REQUEST;
        }
        
        res.set_content(JsonParser::serialize(response), "application/json");
    }
    catch(const std::exception& e)
    {
        spdlog::error("确认告警失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void AlarmHandler::handleResolveAlarm(const httplib::Request& req, httplib::Response& res)
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

        // 获取告警ID
        int alarmId = std::stoi(req.matches[1]);
        
        // 解析请求体
        auto requestBody = JsonParser::parse(req.body);
        std::string remark = requestBody.value("remark", "");
        std::string resolvedBy = requestBody.value("resolvedBy", "admin");
        
        // 解决告警
        bool success = alarmRepo->resolveAlarm(alarmId, remark, resolvedBy);
        
        // 构建响应
        nlohmann::json response;
        if(success)
        {
            response["success"] = true;
            response["message"] = "告警解决成功";
            res.status = Constants::RESPONSE_SUCCESS;
        }
        else
        {
            response["success"] = false;
            response["message"] = "告警解决失败";
            res.status = Constants::RESPONSE_BAD_REQUEST;
        }
        
        res.set_content(JsonParser::serialize(response), "application/json");
    }
    catch(const std::exception& e)
    {
        spdlog::error("解决告警失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}

void AlarmHandler::handleBatchProcessAlarms(const httplib::Request& req, httplib::Response& res)
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
        auto requestBody = JsonParser::parse(req.body);
        std::vector<int> alarmIds = requestBody["alarmIds"].get<std::vector<int>>();
        std::string action = requestBody.value("action", "");
        std::string remark = requestBody.value("remark", "");
        std::string operator_name = requestBody.value("operator", "admin");
        
        // 批量处理告警
        bool success = alarmRepo->batchProcessAlarms(alarmIds, action, remark, operator_name);
        
        // 构建响应
        nlohmann::json response;
        if(success)
        {
            response["success"] = true;
            response["message"] = "批量处理告警成功";
            res.status = Constants::RESPONSE_SUCCESS;
        }
        else
        {
            response["success"] = false;
            response["message"] = "批量处理告警失败";
            res.status = Constants::RESPONSE_BAD_REQUEST;
        }
        
        res.set_content(JsonParser::serialize(response), "application/json");
    }
    catch(const std::exception& e)
    {
        spdlog::error("批量处理告警失败: {}", e.what());
        res.status = Constants::RESPONSE_INTERNAL_ERROR;
        res.set_content(R"({"error": "服务器内部错误"})", "application/json");
    }
}
