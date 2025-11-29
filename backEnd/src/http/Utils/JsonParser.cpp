#include "JsonParser.h"

nlohmann::json JsonParser::parse(const std::string& jsonString) {
    return nlohmann::json::parse(jsonString);
}

std::string JsonParser::serialize(const nlohmann::json& jsonObject) {
    return jsonObject.dump();
}

bool JsonParser::isValid(const std::string& jsonString) {
    try {
        auto result = nlohmann::json::parse(jsonString); // 显式接收返回值
        (void)result; // 显式忽略返回值，消除未使用变量警告
        return true;
    } catch (...) {
        return false;
    }
}
