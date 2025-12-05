#include "Alarm.h"

Alarm::Alarm()
{
    this->id = 0;
    this->device_id = 0;
    this->type = "";
    this->level = "HIGH";
    this->status = "PENDING";
    this->description = "";
    this->value = 0.0;
    this->threshold = 0.0;
    this->occurred_at = std::time(nullptr);
    this->confirmed_at = 0;
    this->resolved_at = 0;
    this->remark = "";
    this->confirmed_by = "";
    this->resolved_by = "";
}

Alarm::~Alarm()
{
    // Destructor implementation if needed
}

// Getters and setters implementation
int Alarm::getId() const { return id; }
void Alarm::setId(int id) { this->id = id; }

int Alarm::getDeviceId() const { return device_id; }
void Alarm::setDeviceId(int device_id) { this->device_id = device_id; }

std::string Alarm::getType() const { return type; }
void Alarm::setType(const std::string& type) { this->type = type; }

std::string Alarm::getLevel() const { return level; }
void Alarm::setLevel(const std::string& level) { this->level = level; }

std::string Alarm::getStatus() const { return status; }
void Alarm::setStatus(const std::string& status) { this->status = status; }

std::string Alarm::getDescription() const { return description; }
void Alarm::setDescription(const std::string& description) { this->description = description; }

double Alarm::getValue() const { return value; }
void Alarm::setValue(double value) { this->value = value; }

double Alarm::getThreshold() const { return threshold; }
void Alarm::setThreshold(double threshold) { this->threshold = threshold; }

std::time_t Alarm::getOccurredAt() const { return occurred_at; }
void Alarm::setOccurredAt(std::time_t occurred_at) { this->occurred_at = occurred_at; }

std::time_t Alarm::getConfirmedAt() const { return confirmed_at; }
void Alarm::setConfirmedAt(std::time_t confirmed_at) { this->confirmed_at = confirmed_at; }

std::time_t Alarm::getResolvedAt() const { return resolved_at; }
void Alarm::setResolvedAt(std::time_t resolved_at) { this->resolved_at = resolved_at; }

std::string Alarm::getRemark() const { return remark; }
void Alarm::setRemark(const std::string& remark) { this->remark = remark; }

std::string Alarm::getConfirmedBy() const { return confirmed_by; }
void Alarm::setConfirmedBy(const std::string& confirmed_by) { this->confirmed_by = confirmed_by; }

std::string Alarm::getResolvedBy() const { return resolved_by; }
void Alarm::setResolvedBy(const std::string& resolved_by) { this->resolved_by = resolved_by; }
