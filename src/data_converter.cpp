#include "data_converter.h"
#include <chrono>
#include <iomanip>
#include <sstream>

std::string DataConverter::sensorDataToJson(const SensorData& data) {
    std::ostringstream json;
    json << "{";
    json << "\"timestamp\":\"" << timestampToIso8601(data.timestamp) << "\",";
    json << "\"cpu_temperature\":" << formatDouble(data.cpu_temperature, 2) << ",";
    json << "\"compass_heading\":" << formatDouble(data.compass_heading, 2) << ",";
    json << "\"gps\":{";
    json << "\"latitude\":" << formatDouble(data.gps_latitude, 6) << ",";
    json << "\"longitude\":" << formatDouble(data.gps_longitude, 6) << ",";
    json << "\"altitude\":" << formatDouble(data.gps_altitude, 2);
    json << "}";
    json << "}";
    return json.str();
}

std::string DataConverter::cpuTemperatureToJson(const SensorData& data) {
    std::ostringstream json;
    json << "{";
    json << "\"timestamp\":\"" << timestampToIso8601(data.timestamp) << "\",";
    json << "\"temperature\":" << formatDouble(data.cpu_temperature, 2) << ",";
    json << "\"unit\":\"celsius\"";
    json << "}";
    return json.str();
}

std::string DataConverter::compassToJson(const SensorData& data) {
    std::ostringstream json;
    json << "{";
    json << "\"timestamp\":\"" << timestampToIso8601(data.timestamp) << "\",";
    json << "\"heading\":" << formatDouble(data.compass_heading, 2) << ",";
    json << "\"unit\":\"degrees\"";
    json << "}";
    return json.str();
}

std::string DataConverter::gpsToJson(const SensorData& data) {
    std::ostringstream json;
    json << "{";
    json << "\"timestamp\":\"" << timestampToIso8601(data.timestamp) << "\",";
    json << "\"latitude\":" << formatDouble(data.gps_latitude, 6) << ",";
    json << "\"longitude\":" << formatDouble(data.gps_longitude, 6) << ",";
    json << "\"altitude\":" << formatDouble(data.gps_altitude, 2) << ",";
    json << "\"unit\":\"decimal_degrees\"";
    json << "}";
    return json.str();
}

std::string DataConverter::timestampToIso8601(const std::chrono::system_clock::time_point& timestamp) {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return oss.str();
}

std::string DataConverter::formatDouble(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

std::string DataConverter::escapeJsonString(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.length());
    
    for (char c : str) {
        switch (c) {
            case '"':  escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b";  break;
            case '\f': escaped += "\\f";  break;
            case '\n': escaped += "\\n";  break;
            case '\r': escaped += "\\r";  break;
            case '\t': escaped += "\\t";  break;
            default:   escaped += c;       break;
        }
    }
    
    return escaped;
} 