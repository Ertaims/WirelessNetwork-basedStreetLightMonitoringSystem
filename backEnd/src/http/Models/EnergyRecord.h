#pragma once
#include <string>
#include <iostream>
#include <ctime>

class EnergyRecord
{
public:
    EnergyRecord();
    ~EnergyRecord();

    // Getters and setters for all fields
    int getId() const; void setId(int id);
    int getDeviceId() const; void setDeviceId(int device_id);
    std::string getRecordDate() const; void setRecordDate(const std::string& record_date);
    double getEnergy() const; void setEnergy(double energy);
    double getOperationHours() const; void setOperationHours(double operation_hours);
    double getAvgBrightness() const; void setAvgBrightness(double avg_brightness);
    std::time_t getCreatedAt() const; void setCreatedAt(std::time_t created_at);

private:
    int id;
    int device_id;
    std::string record_date;
    double energy;
    double operation_hours;
    double avg_brightness;
    std::time_t created_at;
};
