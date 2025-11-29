#pragma once
#include <string>
#include <nlohmann/json.hpp>

class JsonParser {
public:
    // 解析JSON字符串
    static nlohmann::json parse(const std::string& jsonString);
    
    // 序列化JSON对象
    static std::string serialize(const nlohmann::json& jsonObject);
    
    // 验证JSON是否有效
    static bool isValid(const std::string& jsonString);
};