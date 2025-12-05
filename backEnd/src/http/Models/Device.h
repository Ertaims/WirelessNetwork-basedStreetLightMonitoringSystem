#pragma once
#include <string>
#include <iostream>
#include <ctime>

class Device
{    
public:
    Device();
    ~Device();

    // Getters and setters for all fields
    int getId() const; void setId(int id);
    std::string getName() const; void setName(const std::string& name);
    std::string getLocation() const; void setLocation(const std::string& location);
    std::string getStatus() const; void setStatus(const std::string& status);
    std::string getPower() const; void setPower(const std::string& power);
    int getBrightness() const; void setBrightness(int brightness);
    double getVoltage() const; void setVoltage(double voltage);
    double getCurrent() const; void setCurrent(double current);
    std::string getGroupName() const; void setGroupName(const std::string& group_name);
    double getLatitude() const; void setLatitude(double latitude);
    double getLongitude() const; void setLongitude(double longitude);
    bool getAutoMode() const; void setAutoMode(bool auto_mode);
    double getVoltageThreshold() const; void setVoltageThreshold(double voltage_threshold);
    double getCurrentThreshold() const; void setCurrentThreshold(double current_threshold);
    std::string getOnTime() const; void setOnTime(const std::string& on_time);
    std::string getOffTime() const; void setOffTime(const std::string& off_time);
    int getUptimeHours() const; void setUptimeHours(int uptime_hours);
    std::time_t getLastMaintenance() const; void setLastMaintenance(std::time_t last_maintenance);
    std::time_t getCreatedAt() const; void setCreatedAt(std::time_t created_at);
    std::time_t getUpdatedAt() const; void setUpdatedAt(std::time_t updated_at);

private:
    int id;
    std::string name;
    std::string location;
    std::string status;
    std::string power;
    int brightness;
    double voltage;
    double current;
    std::string group_name;
    double latitude;
    double longitude;
    bool auto_mode;
    double voltage_threshold;
    double current_threshold;
    std::string on_time;
    std::string off_time;
    int uptime_hours;
    std::time_t last_maintenance;
    std::time_t created_at;
    std::time_t updated_at;
    
};
