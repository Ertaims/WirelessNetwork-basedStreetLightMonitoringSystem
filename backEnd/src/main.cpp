#include "core/LampMonitor.h"

int main() {
    // EMQX服务器地址和客户端ID
    const std::string server_address = "tcp://10.201.40.164:1883";
    const std::string client_id = "LampMonitorClient";
    
    std::cout << "启动LampMonitor..." << std::endl;
    LampMonitor lampMonitor(server_address, client_id);


    if(!lampMonitor.start()) {
        std::cerr << "无法启动LampMonitor." << std::endl;
        return -1;
    }

    // 模拟路灯控制
    std::cout << "等待5秒后开始模拟控制..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    std::cout << "======= 开始测试路灯控制 =======" << std::endl;
    // 模拟控制命令
    lampMonitor.controlLamp("switch", "ON", 'A', "lamp_001", 100);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "===============================" << std::endl;
    
    lampMonitor.controlLamp("switch", "OFF", 'A', "lamp_002");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // lampMonitor.controlLamp("lamp_001", "OFF");

    // 保持程序运行
    std::cin.get();

    std::cout << "====== 暂停客户端连接 ======" << std::endl;
    lampMonitor.stop();
    std::cout << "===========================" << std::endl;
    return 0;
}