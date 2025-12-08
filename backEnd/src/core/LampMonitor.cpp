#include "LampMonitor.h"
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>

// 初始化静态实例指针
LampMonitor* LampMonitor::instance = nullptr;

std::mutex g_cout_mutex; // 全局互斥锁保护输出

// 前时间的格式化字符串 "YYYY-MM-DD HH:MM:SS"
static std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();                        // 获取当前时间点
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);      // 转换为time_t类型
    std::tm* now_tm = std::localtime(&now_c);                           // 转换为本地时间结构体
    std::ostringstream oss;                                             // 创建字符串流
    oss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");                  // 格式化时间
    return oss.str();
}

// 私有构造函数
LampMonitor::LampMonitor() : mqtt_client(nullptr), isInitialized(false) {
}

// 析构函数
LampMonitor::~LampMonitor() {
    if (isInitialized) {
        stop();
    }
    if (mqtt_client != nullptr) {
        delete mqtt_client;
        mqtt_client = nullptr;
    }
}

// 获取单例实例
LampMonitor& LampMonitor::getInstance() {
    if (instance == nullptr) {
        instance = new LampMonitor();
    }
    return *instance;
}

// 初始化方法
bool LampMonitor::initialize(const std::string& server_address, const std::string& client_id) {
    if (isInitialized) {
        return true; // 已初始化
    }
    
    serverAddress = server_address;
    clientID = client_id;
    
    // 动态创建MQTT客户端
    mqtt_client = new MQTTClientCpp(server_address, client_id);
    
    // 设置消息回调函数
    mqtt_client->setMessageCallback(
        [this](const std::string& topic, const std::string& payload) {
            this->onMessageReceived(topic, payload);
        });
    
    isInitialized = true;
    return true;
}

bool LampMonitor::start()
{
    if (!isInitialized) {
        return false;
    }
    
    // 连接到MQTT服务器
    if(!mqtt_client->connect()) {
        return false;
    }

    std::cout << "=========== 订阅主题 ===========" << std::endl;
    // 订阅灯控主题
    if(!mqtt_client->subscribe(lamp_control_topic + "+")) {
        return false;
    }

    if(!mqtt_client->subscribe(lamp_status_topic + "+")) {
        return false;
    }

    std::cout << "LampMonitor已启动并订阅相关控制主题." << std::endl;
    std::cout << "===============================" << std::endl;
    return true;
}

void LampMonitor::stop()
{
    std::cout << "停止LampMonitor..." << std::endl;
    mqtt_client->disconnect();
    std::cout << "LampMonitor已停止." << std::endl;
}

void LampMonitor::controlLamp(const std::string& command, const std::string &value, const char group, const std::string& lamp_id, int brightness)
{
    json control_msg;
    control_msg["command"] = command;
    control_msg["value"] = value;
    control_msg["group"] = std::string(1, group);
    control_msg["lamp_id"] = lamp_id;
    control_msg["timestamp"] = getCurrentTimestamp();

    if (value == "ON")
    {
        control_msg["brightness"] = brightness;
    }
    else
    {
        control_msg["brightness"] = 0;
    }
    
    std::string topic = lamp_control_topic + lamp_id;
    mqtt_client->publish(topic, control_msg.dump());
}

void LampMonitor::onMessageReceived(const std::string &topic, const std::string &payload)
{
    std::lock_guard<std::mutex> lock(g_cout_mutex);

    std::cout << "收到消息 [主题: " << topic << ", 内容: " << payload << "]" << std::endl;

    try
    {
        json msg = json::parse(payload);

         // 根据 topic 前缀区分处理逻辑，避免直接访问不存在的字段
        if (topic.rfind("lamp/control/", 0) == 0) {
            std::string lamp_id = msg.value("lamp_id", "");
            std::string command = msg.value("command", "");
            std::string value_ = msg.value("value", "");
            int brightness = msg.value("brightness", 0);

            std::cout << "控制灯 " << lamp_id << " 执行命令: " << command << " 亮度: " << brightness << std::endl;
            //publishLampStatus(lamp_id, value_, brightness);
        }
        else if (topic.rfind("lamp/status/", 0) == 0) {command
            std::string lamp_id = msg.value("lamp_id", "");
            std::string status = msg.value("status", "");
            int brightness = msg.value("brightness", 0);

            std::cout << "收到状态消息 for " << lamp_id << " 状态: " << status << " 亮度: " << brightness << std::endl;
        }
        else {
            std::cout << "[WARN] 未知主题: " << topic << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "处理消息时出错: " << e.what() << std::endl;
    }
}

void LampMonitor::publishLampStatus(const std::string& lamp_id, const std::string& status, int brightness)
{
    json status_msg;
    status_msg["lamp_id"] = lamp_id;
    status_msg["status"] = status;
    status_msg["brightness"] = brightness;
    status_msg["timestamp"] = getCurrentTimestamp();

    std::string topic = lamp_status_topic + lamp_id;
    mqtt_client->publish(topic, status_msg.dump());
}
