#include "Device.h"

Device::Device() : 
    id(0), 
    name(""), 
    location(""), 
    status("ONLINE"), 
    power("OFF"), 
    brightness(0), 
    voltage(0.0), 
    current(0.0), 
    group_name(""), 
    latitude(0.0), 
    longitude(0.0), 
    auto_mode(false), 
    voltage_threshold(240.0), 
    current_threshold(1.5), 
    on_time("18:00"), 
    off_time("06:00"), 
    uptime_hours(0), 
    last_maintenance(0), 
    created_at(std::time(nullptr)), 
    updated_at(std::time(nullptr)) {}

Device::~Device() {}

// Getters and setters
int Device::getId() const{ return id; }
void Device::setId(int id) { this->id = id; }

std::string Device::getName() const { return name; }
void Device::setName(const std::string& name) { this->name = name; }

std::string Device::getLocation() const { return location; }
void Device::setLocation(const std::string& location) { this->location = location; }

std::string Device::getStatus() const { return status; }
void Device::setStatus(const std::string& status) { this->status = status; }

std::string Device::getPower() const { return power; }
void Device::setPower(const std::string& power) { this->power = power; }

int Device::getBrightness() const { return brightness; }
void Device::setBrightness(int brightness) { this->brightness = brightness; }

double Device::getVoltage() const { return voltage; }
void Device::setVoltage(double voltage) { this->voltage = voltage; }

double Device::getCurrent() const { return current; }
void Device::setCurrent(double current) { this->current = current; }

std::string Device::getGroupName() const { return group_name; }
void Device::setGroupName(const std::string& group_name) { this->group_name = group_name; }

double Device::getLatitude() const { return latitude; }
void Device::setLatitude(double latitude) { this->latitude = latitude; }

double Device::getLongitude() const { return longitude; }
void Device::setLongitude(double longitude) { this->longitude = longitude; }

bool Device::getAutoMode() const { return auto_mode; }
void Device::setAutoMode(bool auto_mode) { this->auto_mode = auto_mode; }

double Device::getVoltageThreshold() const { return voltage_threshold; }
void Device::setVoltageThreshold(double voltage_threshold) { this->voltage_threshold = voltage_threshold; }

double Device::getCurrentThreshold() const { return current_threshold; }
void Device::setCurrentThreshold(double current_threshold) { this->current_threshold = current_threshold; }

std::string Device::getOnTime() const { return on_time; }
void Device::setOnTime(const std::string& on_time) { this->on_time = on_time; }

std::string Device::getOffTime() const { return off_time; }
void Device::setOffTime(const std::string& off_time) { this->off_time = off_time; }

int Device::getUptimeHours() const { return uptime_hours; }
void Device::setUptimeHours(int uptime_hours) { this->uptime_hours = uptime_hours; }

std::time_t Device::getLastMaintenance() const { return last_maintenance; }
void Device::setLastMaintenance(std::time_t last_maintenance) { this->last_maintenance = last_maintenance; }

std::time_t Device::getCreatedAt() const { return created_at; }
void Device::setCreatedAt(std::time_t created_at) { this->created_at = created_at; }

std::time_t Device::getUpdatedAt() const { return updated_at; }
void Device::setUpdatedAt(std::time_t updated_at) { this->updated_at = updated_at; }

