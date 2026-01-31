# WirelessNetwork-basedStreetLightMonitoringSystem
## 基于无线网络的路灯监控系统

后端使用：
1.部署MQTT Broker（EMQX）
·下载并安装EMQX 5.3.2
链接：Directory listing for EMQX: /v5.3.2/ | EMQ
·启动服务：进入emqx5.3.2/bin
终端输入：emqx start
浏览器输入localhost:18083
在登录页面输入初始化账号 ：用户名：admin 密码：public
2.部署后端服务
要求：
·操作系统: Ubuntu 20.04 LTS或更高版本
·编译器: GCC 9.0或更高版本，支持C++17标准
·构建工具: CMake 3.12或更高版本

首先，安装相关依赖：
·Paho MQTT C库: libpaho-mqtt-dev
·Paho MQTT C++库: libpaho-mqttpp-dev
·httplib: 用于HTTP服务器
·nlohmann/json: 用于JSON处理
·spdlog: 用于日志记录
·MySQL客户端库: libmysqlclient-dev
·fmt库: 用于格式化
·OpenSSL: 用于加密

- 更新系统包
sudo apt update && sudo apt upgrade -y
- 安装基本依赖
sudo apt install -y build-essential cmake git pkg-config
- 安装MQTT相关依赖
sudo apt install -y libpaho-mqtt-dev libpaho-mqttpp-dev
- 安装MySQL客户端库
sudo apt install -y libmysqlclient-dev
- 安装JSON库
sudo apt install -y nlohmann-json3-dev
- 安装spdlog日志库
sudo apt install -y libspdlog-dev
- 安装fmt库
sudo apt install -y libfmt-dev
- 安装OpenSSL
sudo apt install -y libssl-dev
---
克隆项目代码：
``
git clone https://github.com/Ertaims/WirelessNetwork-basedStreetLightMonitoringSystem.git
cd WirelessNetwork-basedStreetLightMonitoringSystem/backEnd
``

构建启动项目：
``
mkdir build
cd build
cmake ..
Make
./streetlight-monitor-backend.exe
``
