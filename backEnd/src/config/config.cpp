#include "config.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

Config* Config::instance = nullptr;

Config& Config::getInstance() {
    if (!instance) {
        instance = new Config();
    }
    return *instance;
}

Config::Config() {
    // 默认配置
    config_data = {
        {"server", {{"port", 8080}, {"host", "0.0.0.0"}}},
        {"database", {
            {"host", "10.201.40.164"},
            {"port", 3306},
            {"username", "root"},
            {"password", "123456"},
            {"database", "streetlight_db"}
        }},
        {"mqtt", {
            {"broker_address", "tcp://localhost:1883"},
            {"client_id", "streetlight_backend"}
        }},
        {"log_level", "INFO"}
    };
}

bool Config::loadFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Cannot open config file: " << filename << std::endl;
            return false;
        }
        
        json file_config;
        file >> file_config;
        
        // 合并配置，文件中的配置覆盖默认配置
        config_data.merge_patch(file_config);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config file: " << e.what() << std::endl;
        return false;
    }
}

// 实现各个getter方法
int Config::getServerPort() const {
    return config_data["server"]["port"];
}

std::string Config::getServerHost() const {
    return config_data["server"]["host"];
}

std::string Config::getDatabaseHost() const {
    return config_data["database"]["host"];
}

int Config::getDatabasePort() const {
    return config_data["database"]["port"];
}

std::string Config::getDatabaseUsername() const {
    return config_data["database"]["username"];
}

std::string Config::getDatabasePassword() const {
    return config_data["database"]["password"];
}

std::string Config::getDatabaseName() const {
    return config_data["database"]["database"];
}

std::string Config::getMqttBrokerAddress() const {
    return config_data["mqtt"]["broker_address"];
}

std::string Config::getMqttClientId() const {
    return config_data["mqtt"]["client_id"];
}

std::string Config::getLogLevel() const {
    return config_data["log_level"];
}