#pragma once

#include "sensor_simulator.h"
#include <string>
#include <sstream>
#include <iomanip>

class DataConverter {
public:
    DataConverter() = default;
    ~DataConverter() = default;

    // Convert sensor data to JSON format
    static std::string sensorDataToJson(const SensorData& data);
    
    // Convert individual sensor data to JSON
    static std::string cpuTemperatureToJson(const SensorData& data);
    static std::string compassToJson(const SensorData& data);
    static std::string gpsToJson(const SensorData& data);
    
    // Convert timestamp to ISO 8601 format
    static std::string timestampToIso8601(const std::chrono::system_clock::time_point& timestamp);
    
    // Format double values with specified precision
    static std::string formatDouble(double value, int precision = 6);

private:
    // Helper method to escape JSON strings
    static std::string escapeJsonString(const std::string& str);
}; 