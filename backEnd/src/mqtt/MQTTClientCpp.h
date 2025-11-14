#pragma once
#include <iostream>
#include <mqtt/async_client.h>
#include <string>

class MQTTClientCpp : public virtual mqtt::callback {
public:
    // 定义消息回调函数类型
    using MessageCallback = std::function<void(const std::string& topic, const std::string& payload)>;
    
    // 初始化MQTT客户端
    MQTTClientCpp(const std::string& server_address, const std::string& client_id);

    // 销毁MQTT客户端
    ~MQTTClientCpp();

    // 连接到MQTT Broker
    bool connect();

    // 断开与MQTT Broker的连接
    void disconnect();

    // 发布话题
    bool publish(const std::string& topic, const std::string& payload, int qos = 1);
    
    // 订阅话题
    bool subscribe(const std::string& topic, int qos = 1);

    // 设置消息回调函数
    void setMessageCallback(MessageCallback callback);

/******************************************************************************************/
    // 回调函数：连接成功
    void connected(const std::string& cause) override;

    // 回调函数：连接丢失
    void connection_lost(const std::string& cause) override;

    // 回调函数：消息到达
    void message_arrived(mqtt::const_message_ptr msg) override;

    // 回调函数：消息投递完成
    void delivery_complete(mqtt::delivery_token_ptr tok) override;

private:
    mqtt::async_client client; // MQTT异步客户端
    MessageCallback message_callback; // 消息回调函数
    std::string client_id; // 客户端ID
};