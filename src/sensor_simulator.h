#pragma once

#include <random>
#include <chrono>
#include <string>

struct SensorData {
    double cpu_temperature;  // in Celsius
    double compass_heading;  // in degrees (0-360)
    double gps_latitude;     // in decimal degrees
    double gps_longitude;    // in decimal degrees
    double gps_altitude;     // in meters
    std::chrono::system_clock::time_point timestamp;
};

class SensorSimulator {
public:
    SensorSimulator();
    ~SensorSimulator() = default;

    // Generate simulated sensor data
    SensorData generateSensorData();

    // Set simulation parameters
    void setCpuTemperatureRange(double min, double max);
    void setCompassVariation(double variation);
    void setGpsDrift(double drift_meters_per_second);
    void setUpdateInterval(int milliseconds);

private:
    // Random number generation
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_real_distribution<double> cpu_temp_dist_;
    std::uniform_real_distribution<double> compass_dist_;
    std::uniform_real_distribution<double> gps_noise_dist_;

    // Simulation parameters
    double cpu_temp_min_;
    double cpu_temp_max_;
    double compass_variation_;
    double gps_drift_;
    int update_interval_ms_;

    // Current simulated position (for GPS drift)
    double current_lat_;
    double current_lon_;
    double current_alt_;

    // Time tracking
    std::chrono::system_clock::time_point last_update_;

    // Helper methods
    double simulateCpuTemperature();
    double simulateCompassHeading();
    void simulateGpsPosition();
    void updateSimulationTime();
}; 