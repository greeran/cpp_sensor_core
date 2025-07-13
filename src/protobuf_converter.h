#pragma once

#include "sensor_simulator.h"
#include "sensor.pb.h"
#include <string>
#include <memory>

class ProtobufConverter {
public:
    ProtobufConverter() = default;
    ~ProtobufConverter() = default;

    // Convert sensor data to protobuf messages
    static std::string sensorDataToProtobuf(const SensorData& data, const std::string& device_id = "imx8mp_sensor");
    static std::string temperatureToProtobuf(const SensorData& data, const std::string& device_id = "imx8mp_sensor");
    static std::string compassToProtobuf(const SensorData& data, const std::string& device_id = "imx8mp_sensor");
    static std::string gpsToProtobuf(const SensorData& data, const std::string& device_id = "imx8mp_sensor");
    
    // Create status messages
    static std::string createOnlineStatus(const std::string& device_id = "imx8mp_sensor");
    static std::string createOfflineStatus(const std::string& device_id = "imx8mp_sensor");
    static std::string createErrorStatus(const std::string& message, const std::string& device_id = "imx8mp_sensor");
    
    // Convert timestamp to Unix milliseconds
    static int64_t timestampToUnixMs(const std::chrono::system_clock::time_point& timestamp);
    
    // Validate protobuf message
    static bool validateMessage(const std::string& serialized_data);

private:
    // Helper methods
    static void setCommonFields(sensor::SensorData& msg, const SensorData& data, const std::string& device_id);
    static void setCommonFields(sensor::TemperatureData& msg, const SensorData& data, const std::string& device_id);
    static void setCommonFields(sensor::CompassData& msg, const SensorData& data, const std::string& device_id);
    static void setCommonFields(sensor::GpsPositionData& msg, const SensorData& data, const std::string& device_id);
}; 