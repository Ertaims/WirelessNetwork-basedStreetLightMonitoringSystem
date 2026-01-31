当然可以！以下是使用 Markdown 语法整理后的文档内容，结构清晰、格式规范，便于阅读和维护：

---

# 基于无线网络的路灯监控系统（Wireless Network-based Street Light Monitoring System）

## 后端部署指南

### 1. 部署 MQTT Broker（EMQX）

#### 步骤：
1. **下载并安装 EMQX 5.3.2**  
   下载地址：[EMQX v5.3.2 目录](https://www.emqx.com/zh/downloads/broker/v5.3.2)

2. **启动 EMQX 服务**
   ```bash
   cd emqx-5.3.2/bin
   ./emqx start
   ```

3. **访问管理界面**  
   在浏览器中打开：  
   [http://localhost:18083](http://localhost:18083)  
   - 默认用户名：`admin`  
   - 默认密码：`public`

---

### 2. 部署后端服务

#### 系统要求：
- **操作系统**：Ubuntu 20.04 LTS 或更高版本  
- **编译器**：GCC 9.0+（需支持 C++17 标准）  
- **构建工具**：CMake 3.12+

#### 安装依赖项

```bash
# 更新系统
sudo apt update && sudo apt upgrade -y

# 安装基础构建工具
sudo apt install -y build-essential cmake git pkg-config

# 安装 MQTT 相关库
sudo apt install -y libpaho-mqtt-dev libpaho-mqttpp-dev

# 安装 MySQL 客户端库
sudo apt install -y libmysqlclient-dev

# 安装 JSON 处理库
sudo apt install -y nlohmann-json3-dev

# 安装日志库
sudo apt install -y libspdlog-dev

# 安装格式化库
sudo apt install -y libfmt-dev

# 安装加密库
sudo apt install -y libssl-dev
```

#### 克隆项目代码

```bash
git clone https://github.com/Ertaims/WirelessNetwork-basedStreetLightMonitoringSystem.git
cd WirelessNetwork-basedStreetLightMonitoringSystem/backEnd
```

#### 构建并运行项目

```bash
mkdir build
cd build
cmake ..
make
./streetlight-monitor-backend.exe
```

> **注意**：若生成的可执行文件名不包含 `.exe`（Linux 系统通常无此扩展），请直接运行：
> ```bash
> ./streetlight-monitor-backend
> ```

--- 

✅ 至此，后端服务应已成功部署并运行。确保 EMQX 已启动且网络配置正确，以便后端能正常连接 MQTT Broker 和数据库。

当然可以！以下是将您提供的内容整理为规范 Markdown 格式的文档：

---

### 3. 初始化数据库

项目包含两个 SQL 脚本文件，位于 `mysql/` 目录下：

- **建表脚本**：用于创建所需的数据库表结构  
- **初始数据脚本**：用于插入系统运行所需的原始数据

#### 操作步骤：

1. 登录 MySQL：
   ```bash
   mysql -u root -p
   ```

2. 创建数据库（例如命名为 `streetlight_db`）：
   ```sql
   CREATE DATABASE IF NOT EXISTS streetlight_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
   USE streetlight_db;
   ```

3. 使用mysql文件夹下两个sql文件，其中一个用于建表，一个用于创建原始数据进行测试

---

### 4. 运行前端

进入前端目录并启动开发服务器：

```bash
cd frontEnd/
```

确保已安装 [Node.js](https://nodejs.org/) 和 `npm`，然后执行：

```bash
# 安装前端依赖
npm install

# 启动开发服务器
npm run dev
```

启动成功后，终端会显示访问地址（通常为 `http://localhost:5173` 或类似）。  
在浏览器中打开该 URL 即可访问前端界面。

> 💡 提示：具体端口号可能因项目配置而异，请以控制台输出为准。
