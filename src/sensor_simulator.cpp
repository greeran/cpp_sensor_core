#include "sensor_simulator.h"
#include <cmath>
#include <iostream>

SensorSimulator::SensorSimulator() 
    : gen_(rd_())
    , cpu_temp_dist_(35.0, 85.0)  // Realistic CPU temp range
    , compass_dist_(0.0, 360.0)
    , gps_noise_dist_(-0.00001, 0.00001)  // Small GPS noise
    , cpu_temp_min_(35.0)
    , cpu_temp_max_(85.0)
    , compass_variation_(5.0)  // degrees
    , gps_drift_(0.1)  // meters per second
    , update_interval_ms_(1000)
    , current_lat_(37.7749)  // San Francisco coordinates as starting point
    , current_lon_(-122.4194)
    , current_alt_(100.0)
    , last_update_(std::chrono::system_clock::now())
{
}

SensorData SensorSimulator::generateSensorData() {
    updateSimulationTime();
    
    SensorData data;
    data.cpu_temperature = simulateCpuTemperature();
    data.compass_heading = simulateCompassHeading();
    simulateGpsPosition();
    data.gps_latitude = current_lat_;
    data.gps_longitude = current_lon_;
    data.gps_altitude = current_alt_;
    data.timestamp = std::chrono::system_clock::now();
    
    return data;
}

void SensorSimulator::setCpuTemperatureRange(double min, double max) {
    cpu_temp_min_ = min;
    cpu_temp_max_ = max;
    cpu_temp_dist_ = std::uniform_real_distribution<double>(min, max);
}

void SensorSimulator::setCompassVariation(double variation) {
    compass_variation_ = variation;
}

void SensorSimulator::setGpsDrift(double drift_meters_per_second) {
    gps_drift_ = drift_meters_per_second;
}

void SensorSimulator::setUpdateInterval(int milliseconds) {
    update_interval_ms_ = milliseconds;
}

double SensorSimulator::simulateCpuTemperature() {
    // Simulate realistic CPU temperature with some variation
    double base_temp = cpu_temp_dist_(gen_);
    
    // Add some realistic variation based on time
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    
    // Add sinusoidal variation to simulate load changes
    double variation = 5.0 * std::sin(seconds * 0.1) + 2.0 * std::sin(seconds * 0.05);
    
    return std::max(cpu_temp_min_, std::min(cpu_temp_max_, base_temp + variation));
}

double SensorSimulator::simulateCompassHeading() {
    // Simulate compass heading with gradual changes
    static double current_heading = 0.0;
    
    // Add random variation
    double variation = compass_dist_(gen_) * compass_variation_ / 360.0;
    current_heading += variation;
    
    // Keep heading in 0-360 range
    if (current_heading >= 360.0) current_heading -= 360.0;
    if (current_heading < 0.0) current_heading += 360.0;
    
    return current_heading;
}

void SensorSimulator::simulateGpsPosition() {
    // Simulate GPS drift over time
    auto now = std::chrono::system_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update_).count();
    double time_seconds = time_diff / 1000.0;
    
    // Convert drift from m/s to degrees (approximate)
    // 1 degree latitude ≈ 111,000 meters
    // 1 degree longitude ≈ 111,000 * cos(latitude) meters
    double lat_drift_deg = (gps_drift_ * time_seconds) / 111000.0;
    double lon_drift_deg = (gps_drift_ * time_seconds) / (111000.0 * std::cos(current_lat_ * M_PI / 180.0));
    
    // Add some random noise
    double lat_noise = gps_noise_dist_(gen_);
    double lon_noise = gps_noise_dist_(gen_);
    
    current_lat_ += lat_drift_deg + lat_noise;
    current_lon_ += lon_drift_deg + lon_noise;
    
    // Add small altitude variation
    current_alt_ += gps_noise_dist_(gen_) * 10.0;  // 10x noise for altitude
    
    last_update_ = now;
}

void SensorSimulator::updateSimulationTime() {
    // This method ensures proper time tracking for simulation
    // Currently handled in simulateGpsPosition()
} 