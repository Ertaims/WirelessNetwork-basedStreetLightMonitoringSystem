#pragma once
#include <string>
#include <iostream>
#include <ctime>

class Alarm
{
public:
    Alarm();
    ~Alarm();

    // Getters and setters for all fields
    int getId() const; void setId(int id);
    int getDeviceId() const; void setDeviceId(int device_id);
    std::string getType() const; void setType(const std::string& type);
    std::string getLevel() const; void setLevel(const std::string& level);
    std::string getStatus() const; void setStatus(const std::string& status);
    std::string getDescription() const; void setDescription(const std::string& description);
    double getValue() const; void setValue(double value);
    double getThreshold() const; void setThreshold(double threshold);
    std::time_t getOccurredAt() const; void setOccurredAt(std::time_t occurred_at);
    std::time_t getConfirmedAt() const; void setConfirmedAt(std::time_t confirmed_at);
    std::time_t getResolvedAt() const; void setResolvedAt(std::time_t resolved_at);
    std::string getRemark() const; void setRemark(const std::string& remark);
    std::string getConfirmedBy() const; void setConfirmedBy(const std::string& confirmed_by);
    std::string getResolvedBy() const; void setResolvedBy(const std::string& resolved_by);

private:
    int id;
    int device_id;
    std::string type;
    std::string level;
    std::string status;
    std::string description;
    double value;
    double threshold;
    std::time_t occurred_at;
    std::time_t confirmed_at;
    std::time_t resolved_at;
    std::string remark;
    std::string confirmed_by;
    std::string resolved_by;
};
