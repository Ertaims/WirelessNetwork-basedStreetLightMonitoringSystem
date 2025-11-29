#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Config {
private:
    static Config* instance;
    json config_data;

    Config(); // 私有构造函数

public:
    static Config& getInstance();
    
    // 禁止拷贝
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    bool loadFromFile(const std::string& filename);
    
    // 获取配置值的方法
    int getServerPort() const;
    std::string getServerHost() const;
    std::string getDatabaseHost() const;
    int getDatabasePort() const;
    std::string getDatabaseUsername() const;
    std::string getDatabasePassword() const;
    std::string getDatabaseName() const;
    std::string getMqttBrokerAddress() const;
    std::string getMqttClientId() const;
    std::string getLogLevel() const;
};

#endif // CONFIG_H