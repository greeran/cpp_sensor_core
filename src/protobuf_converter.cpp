#include "protobuf_converter.h"
#include <chrono>
#include <iostream>

std::string ProtobufConverter::sensorDataToProtobuf(const SensorData& data, const std::string& device_id) {
    sensor::SensorData msg;
    setCommonFields(msg, data, device_id);
    
    // Set GPS data
    auto* gps = msg.mutable_gps();
    gps->set_latitude(data.gps_latitude);
    gps->set_longitude(data.gps_longitude);
    gps->set_altitude(data.gps_altitude);
    gps->set_accuracy(5.0); // Default accuracy of 5 meters
    
    std::string serialized;
    if (!msg.SerializeToString(&serialized)) {
        std::cerr << "Failed to serialize sensor data to protobuf" << std::endl;
        return "";
    }
    
    return serialized;
}

std::string ProtobufConverter::temperatureToProtobuf(const SensorData& data, const std::string& device_id) {
    sensor::TemperatureData msg;
    setCommonFields(msg, data, device_id);
    msg.set_temperature(data.cpu_temperature);
    msg.set_unit("celsius");
    
    std::string serialized;
    if (!msg.SerializeToString(&serialized)) {
        std::cerr << "Failed to serialize temperature data to protobuf" << std::endl;
        return "";
    }
    
    return serialized;
}

std::string ProtobufConverter::compassToProtobuf(const SensorData& data, const std::string& device_id) {
    sensor::CompassData msg;
    setCommonFields(msg, data, device_id);
    msg.set_heading(data.compass_heading);
    msg.set_unit("degrees");
    
    std::string serialized;
    if (!msg.SerializeToString(&serialized)) {
        std::cerr << "Failed to serialize compass data to protobuf" << std::endl;
        return "";
    }
    
    return serialized;
}

std::string ProtobufConverter::gpsToProtobuf(const SensorData& data, const std::string& device_id) {
    sensor::GpsPositionData msg;
    setCommonFields(msg, data, device_id);
    
    auto* position = msg.mutable_position();
    position->set_latitude(data.gps_latitude);
    position->set_longitude(data.gps_longitude);
    position->set_altitude(data.gps_altitude);
    position->set_accuracy(5.0); // Default accuracy of 5 meters
    
    msg.set_unit("decimal_degrees");
    
    std::string serialized;
    if (!msg.SerializeToString(&serialized)) {
        std::cerr << "Failed to serialize GPS data to protobuf" << std::endl;
        return "";
    }
    
    return serialized;
}

std::string ProtobufConverter::createOnlineStatus(const std::string& device_id) {
    sensor::StatusMessage msg;
    msg.set_status(sensor::StatusMessage::ONLINE);
    msg.set_device_id(device_id);
    msg.set_timestamp(timestampToUnixMs(std::chrono::system_clock::now()));
    msg.set_message("Sensor simulator online");
    
    std::string serialized;
    if (!msg.SerializeToString(&serialized)) {
        std::cerr << "Failed to serialize online status to protobuf" << std::endl;
        return "";
    }
    
    return serialized;
}

std::string ProtobufConverter::createOfflineStatus(const std::string& device_id) {
    sensor::StatusMessage msg;
    msg.set_status(sensor::StatusMessage::OFFLINE);
    msg.set_device_id(device_id);
    msg.set_timestamp(timestampToUnixMs(std::chrono::system_clock::now()));
    msg.set_message("Sensor simulator offline");
    
    std::string serialized;
    if (!msg.SerializeToString(&serialized)) {
        std::cerr << "Failed to serialize offline status to protobuf" << std::endl;
        return "";
    }
    
    return serialized;
}

std::string ProtobufConverter::createErrorStatus(const std::string& message, const std::string& device_id) {
    sensor::StatusMessage msg;
    msg.set_status(sensor::StatusMessage::ERROR);
    msg.set_device_id(device_id);
    msg.set_timestamp(timestampToUnixMs(std::chrono::system_clock::now()));
    msg.set_message(message);
    
    std::string serialized;
    if (!msg.SerializeToString(&serialized)) {
        std::cerr << "Failed to serialize error status to protobuf" << std::endl;
        return "";
    }
    
    return serialized;
}

int64_t ProtobufConverter::timestampToUnixMs(const std::chrono::system_clock::time_point& timestamp) {
    auto duration = timestamp.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

bool ProtobufConverter::validateMessage(const std::string& serialized_data) {
    sensor::SensorData msg;
    return msg.ParseFromString(serialized_data);
}

// Helper methods
void ProtobufConverter::setCommonFields(sensor::SensorData& msg, const SensorData& data, const std::string& device_id) {
    msg.set_cpu_temperature(data.cpu_temperature);
    msg.set_compass_heading(data.compass_heading);
    msg.set_timestamp(timestampToUnixMs(data.timestamp));
    msg.set_device_id(device_id);
    msg.set_version("1.0");
}

void ProtobufConverter::setCommonFields(sensor::TemperatureData& msg, const SensorData& data, const std::string& device_id) {
    msg.set_timestamp(timestampToUnixMs(data.timestamp));
}

void ProtobufConverter::setCommonFields(sensor::CompassData& msg, const SensorData& data, const std::string& device_id) {
    msg.set_timestamp(timestampToUnixMs(data.timestamp));
}

void ProtobufConverter::setCommonFields(sensor::GpsPositionData& msg, const SensorData& data, const std::string& device_id) {
    msg.set_timestamp(timestampToUnixMs(data.timestamp));
} 