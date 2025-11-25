#pragma once
#include "mqtt/MQTTClientCpp.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class LampMonitor {
public:
    LampMonitor(const std::string& server_address, const std::string& client_id);
    bool start();
    void stop();
    void controlLamp(const std::string& command, const std::string &value, const char group, const std::string& lamp_id, int brightness = 50);

private:
    void onMessageReceived(const std::string& topic, const std::string& payload);
    void publishLampStatus(const std::string& lamp_id, const std::string& status, int brightness);

    MQTTClientCpp mqtt_client;
    std::string clientID;

    // 主题格式
    const std::string lamp_control_topic = "lamp/control/";
    const std::string lamp_status_topic = "lamp/status/";
    
};