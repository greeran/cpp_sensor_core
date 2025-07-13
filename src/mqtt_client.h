#pragma once

#include <mosquitto.h>
#include <string>
#include <functional>
#include <memory>

class MqttClient {
public:
    MqttClient();
    ~MqttClient();

    // Connection management
    bool connect(const std::string& broker, int port = 1883, int keepalive = 60);
    void disconnect();
    bool isConnected() const;

    // Publishing
    bool publish(const std::string& topic, const std::string& message, int qos = 0);
    bool publishRetained(const std::string& topic, const std::string& message, int qos = 0);

    // Configuration
    void setClientId(const std::string& client_id);
    void setUsername(const std::string& username);
    void setPassword(const std::string& password);
    void setWill(const std::string& topic, const std::string& message, int qos = 0);

    // Callbacks
    void setOnConnect(std::function<void(int)> callback);
    void setOnDisconnect(std::function<void(int)> callback);
    void setOnPublish(std::function<void(int)> callback);

    // Loop management
    int loop(int timeout_ms = -1);
    void loopStart();
    void loopStop();

private:
    struct mosquitto* mosq_;
    std::string client_id_;
    std::string username_;
    std::string password_;
    std::string will_topic_;
    std::string will_message_;
    int will_qos_;
    bool connected_;

    // Callbacks
    std::function<void(int)> on_connect_callback_;
    std::function<void(int)> on_disconnect_callback_;
    std::function<void(int)> on_publish_callback_;

    // Static callback functions for mosquitto
    static void onConnect(struct mosquitto* mosq, void* userdata, int rc);
    static void onDisconnect(struct mosquitto* mosq, void* userdata, int rc);
    static void onPublish(struct mosquitto* mosq, void* userdata, int mid);
}; 