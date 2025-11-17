cmake_minimum_required(VERSION 3.12)
project(streetlight-monitor-backend VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 自动包含当前目录和src目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# 查找依赖包
find_package(PkgConfig REQUIRED)

# 查找 Paho MQTT C 库
find_path(PAHO_MQTT_C_INCLUDE_DIRS
    NAMES MQTTClient.h
    PATHS /usr/include /usr/local/include
    PATH_SUFFIXES paho-mqtt
)

find_library(PAHO_MQTT_C_LIBRARIES
    NAMES paho-mqtt3a paho-mqtt3as paho-mqtt3c paho-mqtt3cs
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
)

# 查找 Paho MQTT C++ 库
find_path(PAHO_MQTT_CPP_INCLUDE_DIRS
    NAMES mqtt/async_client.h
    PATHS /usr/include /usr/local/include
)

find_library(PAHO_MQTT_CPP_LIBRARIES
    NAMES paho-mqttpp3
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
)

# 查找 nlohmann_json
find_package(nlohmann_json 3.12.0 QUIET)
if(NOT nlohmann_json_FOUND)
    # 如果没找到，尝试通过其他方式查找
    find_path(JSON_INCLUDE_DIR
        NAMES nlohmann/json.hpp
        PATHS /usr/include /usr/local/include
    )
    if(JSON_INCLUDE_DIR)
        set(nlohmann_json_FOUND TRUE)
        message(STATUS "Found nlohmann_json at: ${JSON_INCLUDE_DIR}")
    else()
        message(FATAL_ERROR "nlohmann_json not found! Please install it: sudo apt-get install nlohmann-json3-dev")
    endif()
endif()

# 查找 Threads
find_package(Threads REQUIRED)

# 检查依赖是否找到
if(NOT PAHO_MQTT_C_LIBRARIES)
    message(WARNING "Paho MQTT C library not found, trying to find package...")
    find_package(PahoMqttC QUIET)
    if(PahoMqttC_FOUND)
        set(PAHO_MQTT_C_LIBRARIES PahoMqttC::PahoMqttC)
        message(STATUS "Found Paho MQTT C via package config")
    else()
        message(FATAL_ERROR "Paho MQTT C libraries not found. Please install: sudo apt-get install libpaho-mqtt-dev")
    endif()
endif()

if(NOT PAHO_MQTT_CPP_LIBRARIES)
    message(WARNING "Paho MQTT C++ library not found, trying to find package...")
    find_package(PahoMqttCpp QUIET)
    if(PahoMqttCpp_FOUND)
        set(PAHO_MQTT_CPP_LIBRARIES PahoMqttCpp::PahoMqttCpp)
        message(STATUS "Found Paho MQTT C++ via package config")
    else()
        message(FATAL_ERROR "Paho MQTT C++ libraries not found. Please install: sudo apt-get install libpaho-mqttpp-dev")
    endif()
endif()

message(STATUS "Found Paho MQTT C: ${PAHO_MQTT_C_LIBRARIES}")
message(STATUS "Found Paho MQTT C++: ${PAHO_MQTT_CPP_LIBRARIES}")

# 添加可执行文件
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/mqtt/MQTTClientCpp.cpp
    src/core/LampMonitor.cpp
)

# 设置目标包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${PAHO_MQTT_C_INCLUDE_DIRS}
    ${PAHO_MQTT_CPP_INCLUDE_DIRS}
    ${JSON_INCLUDE_DIR}
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    ${PAHO_MQTT_CPP_LIBRARIES}
    ${PAHO_MQTT_C_LIBRARIES}
    Threads::Threads
)

# 如果是通过find_package找到的nlohmann_json
if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)
endif()

# Windows 特定设置
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32 crypt32)
endif()

# 设置编译属性
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

message(STATUS "")
message(STATUS "Build configuration complete!")
message(STATUS "  Project: ${PROJECT_NAME}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")