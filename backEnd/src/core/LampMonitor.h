#pragma once
#include "mqtt/MQTTClientCpp.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class LampMonitor {
public:
    // 获取单例实例
    static LampMonitor& getInstance();
    
    // 初始化方法（用于设置服务器地址和客户端ID）
    bool initialize(const std::string& server_address, const std::string& client_id);
    
    bool start();
    void stop();
    void controlLamp(const std::string& command, const std::string &value, const char group, const std::string& lamp_id, int brightness = 50);

private:
    // 私有构造函数，防止外部实例化
    LampMonitor();
    
    // 私有析构函数
    ~LampMonitor();
    
    // 禁用拷贝构造和赋值操作符
    LampMonitor(const LampMonitor&) = delete;
    LampMonitor& operator=(const LampMonitor&) = delete;
    
    void onMessageReceived(const std::string& topic, const std::string& payload);
    void publishLampStatus(const std::string& lamp_id, const std::string& status, int brightness);

    MQTTClientCpp* mqtt_client;
    std::string clientID;
    std::string serverAddress;
    bool isInitialized;
    
    // 主题格式
    const std::string lamp_control_topic = "lamp/control/";
    const std::string lamp_status_topic = "lamp/status/";
    
    // 单例实例
    static LampMonitor* instance;
};