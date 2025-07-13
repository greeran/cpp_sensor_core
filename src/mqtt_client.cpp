#include "mqtt_client.h"
#include <iostream>
#include <cstring>

MqttClient::MqttClient() 
    : mosq_(nullptr)
    , will_qos_(0)
    , connected_(false)
{
    // Initialize mosquitto library
    mosquitto_lib_init();
    
    // Create mosquitto instance
    mosq_ = mosquitto_new(nullptr, true, this);
    if (!mosq_) {
        std::cerr << "Failed to create mosquitto instance" << std::endl;
        return;
    }
    
    // Set callbacks
    mosquitto_connect_callback_set(mosq_, onConnect);
    mosquitto_disconnect_callback_set(mosq_, onDisconnect);
    mosquitto_publish_callback_set(mosq_, onPublish);
}

MqttClient::~MqttClient() {
    if (mosq_) {
        mosquitto_destroy(mosq_);
    }
    mosquitto_lib_cleanup();
}

bool MqttClient::connect(const std::string& broker, int port, int keepalive) {
    if (!mosq_) {
        std::cerr << "Mosquitto instance not initialized" << std::endl;
        return false;
    }
    
    // Set client ID if provided
    if (!client_id_.empty()) {
        mosquitto_username_pw_set(mosq_, username_.c_str(), password_.c_str());
    }
    
    // Set username/password if provided
    if (!username_.empty()) {
        mosquitto_username_pw_set(mosq_, username_.c_str(), password_.c_str());
    }
    
    // Set will message if provided
    if (!will_topic_.empty()) {
        mosquitto_will_set(mosq_, will_topic_.c_str(), will_message_.length(), 
                          will_message_.c_str(), will_qos_, false);
    }
    
    // Connect to broker
    int rc = mosquitto_connect(mosq_, broker.c_str(), port, keepalive);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to MQTT broker: " << mosquitto_strerror(rc) << std::endl;
        return false;
    }
    
    return true;
}

void MqttClient::disconnect() {
    if (mosq_ && connected_) {
        mosquitto_disconnect(mosq_);
        connected_ = false;
    }
}

bool MqttClient::isConnected() const {
    return connected_;
}

bool MqttClient::publish(const std::string& topic, const std::string& message, int qos) {
    if (!mosq_ || !connected_) {
        std::cerr << "Not connected to MQTT broker" << std::endl;
        return false;
    }
    
    int rc = mosquitto_publish(mosq_, nullptr, topic.c_str(), message.length(), 
                              message.c_str(), qos, false);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to publish message: " << mosquitto_strerror(rc) << std::endl;
        return false;
    }
    
    return true;
}

bool MqttClient::publishRetained(const std::string& topic, const std::string& message, int qos) {
    if (!mosq_ || !connected_) {
        std::cerr << "Not connected to MQTT broker" << std::endl;
        return false;
    }
    
    int rc = mosquitto_publish(mosq_, nullptr, topic.c_str(), message.length(), 
                              message.c_str(), qos, true);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to publish retained message: " << mosquitto_strerror(rc) << std::endl;
        return false;
    }
    
    return true;
}

void MqttClient::setClientId(const std::string& client_id) {
    client_id_ = client_id;
    if (mosq_) {
        mosquitto_destroy(mosq_);
        mosq_ = mosquitto_new(client_id_.c_str(), true, this);
        if (mosq_) {
            mosquitto_connect_callback_set(mosq_, onConnect);
            mosquitto_disconnect_callback_set(mosq_, onDisconnect);
            mosquitto_publish_callback_set(mosq_, onPublish);
        }
    }
}

void MqttClient::setUsername(const std::string& username) {
    username_ = username;
}

void MqttClient::setPassword(const std::string& password) {
    password_ = password;
}

void MqttClient::setWill(const std::string& topic, const std::string& message, int qos) {
    will_topic_ = topic;
    will_message_ = message;
    will_qos_ = qos;
}

void MqttClient::setOnConnect(std::function<void(int)> callback) {
    on_connect_callback_ = callback;
}

void MqttClient::setOnDisconnect(std::function<void(int)> callback) {
    on_disconnect_callback_ = callback;
}

void MqttClient::setOnPublish(std::function<void(int)> callback) {
    on_publish_callback_ = callback;
}

int MqttClient::loop(int timeout_ms) {
    if (!mosq_) return MOSQ_ERR_INVAL;
    return mosquitto_loop(mosq_, timeout_ms, 1);
}

void MqttClient::loopStart() {
    if (mosq_) {
        mosquitto_loop_start(mosq_);
    }
}

void MqttClient::loopStop() {
    if (mosq_) {
        mosquitto_loop_stop(mosq_, true);
    }
}

// Static callback functions
void MqttClient::onConnect(struct mosquitto* mosq, void* userdata, int rc) {
    MqttClient* client = static_cast<MqttClient*>(userdata);
    if (rc == 0) {
        client->connected_ = true;
        std::cout << "Connected to MQTT broker successfully" << std::endl;
    } else {
        std::cerr << "Failed to connect to MQTT broker, return code: " << rc << std::endl;
    }
    
    if (client->on_connect_callback_) {
        client->on_connect_callback_(rc);
    }
}

void MqttClient::onDisconnect(struct mosquitto* mosq, void* userdata, int rc) {
    MqttClient* client = static_cast<MqttClient*>(userdata);
    client->connected_ = false;
    std::cout << "Disconnected from MQTT broker" << std::endl;
    
    if (client->on_disconnect_callback_) {
        client->on_disconnect_callback_(rc);
    }
}

void MqttClient::onPublish(struct mosquitto* mosq, void* userdata, int mid) {
    MqttClient* client = static_cast<MqttClient*>(userdata);
    
    if (client->on_publish_callback_) {
        client->on_publish_callback_(mid);
    }
} 