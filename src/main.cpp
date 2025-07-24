#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include <cstring>
#include "sensor_simulator.h"
#include "mqtt_client.h"
#include "protobuf_converter.h"
#include "actions.pb.h"
#include <map>
#include <functional>
#include "action_handler.h"

// Global variables for signal handling
volatile bool running = true;
MqttClient* g_mqtt_client = nullptr;

// Map for action handlers: topic -> handler function
std::map<std::string, std::function<std::string(const std::string&)>> action_handlers;

// Register an action handler
void register_action_handler(const std::string& topic, std::function<std::string(const std::string&)> handler) {
    action_handlers[topic] = handler;
}

// Example handler for 'reboot' action
auto handle_action_reboot = [](const std::string& payload) -> std::string {
    // Do something with payload
    std::cout << "[Handler] Reboot action triggered with payload: " << payload << std::endl;
    return "Rebooted successfully";
};

// Example handler for 'status' action
auto handle_action_message = [](const std::string& payload) -> std::string {
    std::cout << "[Handler] Status action triggered with payload: " << payload << std::endl;
    return "Status: OK";
};

// Signal handler for graceful shutdown
void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down gracefully..." << std::endl;
    running = false;
    if (g_mqtt_client) {
        g_mqtt_client->disconnect();
    }
}

// Print usage information
void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n"
              << "Options:\n"
              << "  -b, --broker HOST[:PORT]    MQTT broker address (default: localhost:1883)\n"
              << "  -i, --interval MS           Update interval in milliseconds (default: 1000)\n"
              << "  -t, --temp-min TEMP         Minimum CPU temperature in Celsius (default: 35.0)\n"
              << "  -T, --temp-max TEMP         Maximum CPU temperature in Celsius (default: 85.0)\n"
              << "  -c, --compass-var DEG       Compass variation in degrees (default: 5.0)\n"
              << "  -g, --gps-drift MPS         GPS drift in meters per second (default: 0.1)\n"
              << "  -u, --username USER         MQTT username\n"
              << "  -p, --password PASS         MQTT password\n"
              << "  -d, --client-id ID          MQTT client ID (default: sensor_simulator)\n"
              << "  -h, --help                  Show this help message\n"
              << "\nMQTT Topics (Protocol Buffers):\n"
              << "  sensor/temperature           CPU temperature data (protobuf)\n"
              << "  sensor/compass              Compass heading data (protobuf)\n"
              << "  sensor/gps                  GPS position data (protobuf)\n"
              << "  sensor/all                  All sensor data combined (protobuf)\n"
              << "  sensor/status               Device status (protobuf)\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    // Default configuration
    std::string broker = "localhost";
    int port = 1883;
    int interval_ms = 1000;
    double temp_min = 35.0;
    double temp_max = 85.0;
    double compass_var = 5.0;
    double gps_drift = 0.1;
    std::string username;
    std::string password;
    std::string client_id = "sensor_simulator";

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-b" || arg == "--broker") {
            if (++i < argc) {
                std::string broker_arg = argv[i];
                size_t colon_pos = broker_arg.find(':');
                if (colon_pos != std::string::npos) {
                    broker = broker_arg.substr(0, colon_pos);
                    port = std::stoi(broker_arg.substr(colon_pos + 1));
                } else {
                    broker = broker_arg;
                }
            }
        } else if (arg == "-i" || arg == "--interval") {
            if (++i < argc) interval_ms = std::stoi(argv[i]);
        } else if (arg == "-t" || arg == "--temp-min") {
            if (++i < argc) temp_min = std::stod(argv[i]);
        } else if (arg == "-T" || arg == "--temp-max") {
            if (++i < argc) temp_max = std::stod(argv[i]);
        } else if (arg == "-c" || arg == "--compass-var") {
            if (++i < argc) compass_var = std::stod(argv[i]);
        } else if (arg == "-g" || arg == "--gps-drift") {
            if (++i < argc) gps_drift = std::stod(argv[i]);
        } else if (arg == "-u" || arg == "--username") {
            if (++i < argc) username = argv[i];
        } else if (arg == "-p" || arg == "--password") {
            if (++i < argc) password = argv[i];
        } else if (arg == "-d" || arg == "--client-id") {
            if (++i < argc) client_id = argv[i];
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "Sensor Simulator for IMX8MP" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << "MQTT Broker: " << broker << ":" << port << std::endl;
    std::cout << "Update Interval: " << interval_ms << "ms" << std::endl;
    std::cout << "CPU Temperature Range: " << temp_min << "°C - " << temp_max << "°C" << std::endl;
    std::cout << "Compass Variation: " << compass_var << "°" << std::endl;
    std::cout << "GPS Drift: " << gps_drift << " m/s" << std::endl;
    std::cout << "Client ID: " << client_id << std::endl;
    std::cout << std::endl;

    // Initialize components
    SensorSimulator simulator;
    MqttClient mqtt_client;
    g_mqtt_client = &mqtt_client;

    // Configure simulator
    simulator.setCpuTemperatureRange(temp_min, temp_max);
    simulator.setCompassVariation(compass_var);
    simulator.setGpsDrift(gps_drift);
    simulator.setUpdateInterval(interval_ms);

    // Configure MQTT client
    mqtt_client.setClientId(client_id);
    if (!username.empty()) {
        mqtt_client.setUsername(username);
        mqtt_client.setPassword(password);
    }
    mqtt_client.setWill("sensor/status", ProtobufConverter::createOfflineStatus(client_id), 1);

    // Instantiate ActionHandler
    ActionHandler action_handler;

    // Register action handlers
    action_handler.register_action_handler("reboot", handle_action_reboot);
    action_handler.register_action_handler("message", handle_action_message);

    // Set up MQTT message handler
    mqtt_client.setOnMessage([&mqtt_client, &action_handler](const std::string& topic, const std::string& payload) {
        actions::ActionRequest req;
        if (req.ParseFromString(payload)) {
            std::string full_topic = req.topic();
            std::string action_topic = full_topic.substr(full_topic.find_last_of('/') + 1);
            std::string ack_topic = req.ack_topic();
            auto [found, result] = action_handler.run_handler(action_topic, req.payload());
            if (found) {
                std::cout << "[MQTT] ActionRequest handled for topic '" << action_topic << "', result: '" << result << "'\n";
            } else {
                std::cout << "[MQTT] No handler found for action topic '" << action_topic << "'\n";
            }
            if (!ack_topic.empty()) {
                std::string ack_full_topic = "action/ack/" + ack_topic;
                actions::ActionAck ack_msg;
                ack_msg.set_ack(action_topic);
                ack_msg.set_success(found);
                ack_msg.set_error(found ? "" : "No handler found");
                ack_msg.set_allocated_ack(new std::string(result));
                std::string ack_payload;
                ack_msg.SerializeToString(&ack_payload);
                mqtt_client.publish(ack_full_topic, ack_payload, 1);
                std::cout << "[MQTT] Published ack to '" << ack_full_topic << "'\n";
            }
        } else {
            std::cout << "[MQTT] Received message on topic '" << topic << "' (unknown action message or parse error)\n";
        }
    });

    // Set up MQTT callbacks
    mqtt_client.setOnConnect([client_id](int rc) {
        if (rc == 0) {
            std::cout << "Connected to MQTT broker successfully" << std::endl;
            // Publish online status
            if (g_mqtt_client) {
                g_mqtt_client->publishRetained("sensor/status", ProtobufConverter::createOnlineStatus(client_id), 1);
                // Subscribe to all actions topics
                g_mqtt_client->subscribe("action/#", 1);
            }
        }
    });

    mqtt_client.setOnDisconnect([](int rc) {
        std::cout << "Disconnected from MQTT broker" << std::endl;
    });

    mqtt_client.setOnPublish([](int mid) {
        // Optional: Add publish confirmation logging here
    });

    // Connect to MQTT broker
    if (!mqtt_client.connect(broker, port)) {
        std::cerr << "Failed to connect to MQTT broker. Exiting." << std::endl;
        return 1;
    }

    // Start MQTT loop in background thread
    mqtt_client.loopStart();

    // Wait for connection
    int retry_count = 0;
    while (!mqtt_client.isConnected() && retry_count < 10) {
        std::cout << "Waiting for MQTT connection... (" << (retry_count + 1) << "/10)" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        retry_count++;
    }

    if (!mqtt_client.isConnected()) {
        std::cerr << "Failed to connect to MQTT broker after 10 attempts. Exiting." << std::endl;
        return 1;
    }

    std::cout << "Starting sensor simulation..." << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    std::cout << std::endl;

    // Main simulation loop
    while (running) {
        try {
            // Generate sensor data
            SensorData data = simulator.generateSensorData();

            // Convert to protobuf and publish
            std::string all_data_pb = ProtobufConverter::sensorDataToProtobuf(data, client_id);
            std::string temp_pb = ProtobufConverter::temperatureToProtobuf(data, client_id);
            std::string compass_pb = ProtobufConverter::compassToProtobuf(data, client_id);
            std::string gps_pb = ProtobufConverter::gpsToProtobuf(data, client_id);

            // Publish to MQTT topics
            mqtt_client.publish("sensor/all", all_data_pb);
            mqtt_client.publish("sensor/temperature", temp_pb);
            mqtt_client.publish("sensor/compass", compass_pb);
            mqtt_client.publish("sensor/gps", gps_pb);

            // Print status
            std::cout << "Published sensor data - "
                      << "CPU: " << data.cpu_temperature << "°C, "
                      << "Compass: " << data.compass_heading << "°, "
                      << "GPS: " << data.gps_latitude << "," << data.gps_longitude
                      << " (" << data.gps_altitude << "m)" << std::endl;

            // Wait for next update
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));

        } catch (const std::exception& e) {
            std::cerr << "Error in simulation loop: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    // Cleanup
    std::cout << "Shutting down..." << std::endl;
    mqtt_client.publishRetained("sensor/status", ProtobufConverter::createOfflineStatus(client_id), 1);
    mqtt_client.disconnect();
    mqtt_client.loopStop();

    std::cout << "Sensor simulator stopped." << std::endl;
    return 0;
} 