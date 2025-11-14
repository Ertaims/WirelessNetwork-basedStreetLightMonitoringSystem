#include "MQTTClientCpp.h"

MQTTClientCpp::MQTTClientCpp(const std::string& server_address, const std::string& client_id)
    : client(server_address, client_id), client_id(client_id)
{   
    // 设置回调函数
    client.set_callback(*this);
}

MQTTClientCpp::~MQTTClientCpp()
{
    disconnect();
}

bool MQTTClientCpp::connect()
{
    try
    {
        // 创建连接选项
        mqtt::connect_options conn_opts;
        conn_opts.set_clean_session(true); // 设置为干净会话
        conn_opts.set_keep_alive_interval(60); // 设置保持连接间隔
        conn_opts.set_automatic_reconnect(true); // 启用自动重连

        // 连接到MQTT Broker
        mqtt::token_ptr conntok = client.connect(conn_opts);
        conntok->wait(); // 等待连接完成

        std::cout << "成功连接到EMQX服务器: " << client.get_server_uri() << std::endl;
        return true;
    }
    catch(const mqtt::exception& e)
    {
        std::cerr << "连接EMQX服务器失败: " << e.what() << std::endl;
        return false;
    }
}

void MQTTClientCpp::disconnect()
{
    try
    {
        // 如果客户端已经不在线，直接返回，避免重复阻塞
        if (!client.is_connected()) {
            std::cerr << "[INFO] client is not connected, skip disconnect()\n";
            return;
        }

        std::cout << "disconnecting from EMQX server..." << std::endl;
        // 使用带超时的等待，避免无限阻塞（等待最多 5000 ms）
        auto tok = client.disconnect();
        if (tok) {
            bool ok = tok->wait_for(5000); // 等待 5000 ms，返回 false 表示超时
            if (!ok) {
                std::cerr << "[WARN] disconnect() timed out after 5000 ms\n";
                // 如果需要，可在这里做额外清理或强制退出连接相关线程
            } else {
                std::cout << "已断开EMQX服务器的连接" << std::endl;
            }
        } else {
            std::cerr << "[WARN] client.disconnect() returned null token\n";
        }
    }
    catch(const mqtt::exception& e)
    {
        std::cerr << "断开连接时出错: " << e.what() << std::endl;
    }
    
}

bool MQTTClientCpp::publish(const std::string& topic, const std::string& payload, int qos)
{
    try
    {   
        std::cout << "========================================================" << std::endl;
        std::cout << "准备发布消息到主题: " << topic << std::endl;
        std::cout << "消息内容: " << payload << std::endl;
        std::cout << "========================================================" << std::endl;

        auto msg = mqtt::make_message(topic, payload); // 创建消息
        msg->set_qos(qos); // 设置QoS等级

        // 发布消息
        auto pubtok = client.publish(msg);
        std::cout << "等待5s，确保消息发布完成..." << std::endl;
        // pubtok->wait(); // 等待发布完成
        pubtok->wait_for(std::chrono::seconds(5)); // 等待最多5秒，避免无限阻塞

        std::cout << "消息发布成功 [主题: " << topic << ", 内容: " << payload << "]" << std::endl;
        return true;
    }
    catch(const mqtt::exception& e)
    {
        std::cerr << "消息发布失败: " << e.what() << std::endl;
        return false;
    }
}

bool MQTTClientCpp::subscribe(const std::string& topic, int qos)
{
    try
    {
        client.subscribe(topic, qos)->wait(); // 等待订阅完成
        std::cout << "订阅成功 [主题: " << topic << "]" << std::endl;
        return true;
    }
    catch(const mqtt::exception& e)
    {
        std::cerr << "订阅失败: " << e.what() << std::endl;
        return false;
    } 
}

void MQTTClientCpp::setMessageCallback(MessageCallback callback)
{
    message_callback = callback;
}

/******************************************************************************************/
void MQTTClientCpp::connected(const std::string &cause)
{
    std::cout << "已连接到EMQX服务器. 原因: " << cause << std::endl;
}

void MQTTClientCpp::connection_lost(const std::string &cause)
{
    std::cerr << "与EMQX服务器的连接丢失. 原因: " << cause << std::endl;
}

void MQTTClientCpp::delivery_complete(mqtt::delivery_token_ptr tok)
{
    std::cout << "消息投递完成. 令牌ID: " << (tok ? tok->get_message_id() : -1) << std::endl;
}

void MQTTClientCpp::message_arrived(mqtt::const_message_ptr msg)
{
    if (message_callback)
    {
        std::string topic = msg->get_topic(); // 获取主题
        std::string payload = msg->get_payload_str(); // 获取消息内容
        // 调用用户定义的回调函数
        message_callback(topic, payload);
    }
}
