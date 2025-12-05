#include "EnergyRecord.h"

EnergyRecord::EnergyRecord()
    : id(0), device_id(0), energy(0.0), operation_hours(0.0), avg_brightness(0.0), created_at(std::time(nullptr))
{}

EnergyRecord::~EnergyRecord()
{}

// Getters and setters implementation
int EnergyRecord::getId() const { return id; }
void EnergyRecord::setId(int id) { this->id = id; }

int EnergyRecord::getDeviceId() const { return device_id; }
void EnergyRecord::setDeviceId(int device_id) { this->device_id = device_id; }

std::string EnergyRecord::getRecordDate() const { return record_date; }
void EnergyRecord::setRecordDate(const std::string& record_date) { this->record_date = record_date; }

double EnergyRecord::getEnergy() const { return energy; }
void EnergyRecord::setEnergy(double energy) { this->energy = energy; }

double EnergyRecord::getOperationHours() const { return operation_hours; }
void EnergyRecord::setOperationHours(double operation_hours) { this->operation_hours = operation_hours; }

double EnergyRecord::getAvgBrightness() const { return avg_brightness; }
void EnergyRecord::setAvgBrightness(double avg_brightness) { this->avg_brightness = avg_brightness; }

std::time_t EnergyRecord::getCreatedAt() const { return created_at; }
void EnergyRecord::setCreatedAt(std::time_t created_at) { this->created_at = created_at; }
