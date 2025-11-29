#pragma once
#include <string>

namespace Constants {
    // 设备状态常量
    const std::string DEVICE_STATUS_ONLINE = "online";
    const std::string DEVICE_STATUS_OFFLINE = "offline";
    const std::string DEVICE_STATUS_FAULT = "fault";

    // 设备电源状态
    const std::string DEVICE_POWER_ON = "on";
    const std::string DEVICE_POWER_OFF = "off";

    // 告警级别
    const std::string ALARM_LEVEL_HIGH = "high";
    const std::string ALARM_LEVEL_MEDIUM = "medium";
    const std::string ALARM_LEVEL_LOW = "low";

    // 告警状态
    const std::string ALARM_STATUS_PENDING = "pending";             // 等待确认
    const std::string ALARM_STATUS_CONFIRMED = "confirmed";         // 已确认
    const std::string ALARM_STATUS_RESOLVED = "resolved";           // 已解决

    // 用户角色
    const std::string USER_ROLE_ADMIN = "admin";                    // 管理员
    const std::string USER_ROLE_OPERATOR = "operator";              // 操作员
    const std::string USER_ROLE_VIEWER = "viewer";                  // 浏览者

    // API响应码
    const int RESPONSE_SUCCESS = 200;                               // 成功
    const int RESPONSE_CREATED = 201;                               // 创建成功
    const int RESPONSE_BAD_REQUEST = 400;                           // 请求错误
    const int RESPONSE_UNAUTHORIZED = 401;                          // 未授权
    const int RESPONSE_FORBIDDEN = 403;                             // 禁止
    const int RESPONSE_NOT_FOUND = 404;                             // 未找到
    const int RESPONSE_INTERNAL_ERROR = 500;                        // 内部错误 

    // JWT相关常量
    const int JWT_EXPIRATION_HOURS = 24; // 令牌有效期24小时
    const int JWT_REFRESH_DAYS = 7; // 刷新令牌有效期7天
}