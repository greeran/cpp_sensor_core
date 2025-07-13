#!/usr/bin/env python3
"""
Protobuf Test Script for IMX8MP Sensor Simulator
This script demonstrates how to decode protobuf messages from MQTT topics.
"""

import paho.mqtt.client as mqtt
import sys
import os

# Add the proto directory to the Python path
sys.path.append(os.path.join(os.path.dirname(__file__), 'proto'))

try:
    import sensor_pb2
except ImportError:
    print("Error: Could not import sensor_pb2")
    print("Please generate the protobuf Python files first:")
    print("  protoc --python_out=proto/ proto/sensor.proto")
    sys.exit(1)

def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with result code {rc}")
    # Subscribe to all sensor topics
    client.subscribe("sensor/#")

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload
    
    print(f"\n--- Message from {topic} ---")
    
    try:
        if topic == "sensor/all":
            sensor_data = sensor_pb2.SensorData()
            sensor_data.ParseFromString(payload)
            print(f"Combined Sensor Data:")
            print(f"  CPU Temperature: {sensor_data.cpu_temperature}°C")
            print(f"  Compass Heading: {sensor_data.compass_heading}°")
            print(f"  GPS: {sensor_data.gps.latitude}, {sensor_data.gps.longitude}")
            print(f"  Altitude: {sensor_data.gps.altitude}m")
            print(f"  Device ID: {sensor_data.device_id}")
            print(f"  Version: {sensor_data.version}")
            print(f"  Timestamp: {sensor_data.timestamp}")
            
        elif topic == "sensor/temperature":
            temp_data = sensor_pb2.TemperatureData()
            temp_data.ParseFromString(payload)
            print(f"Temperature Data:")
            print(f"  Temperature: {temp_data.temperature}°C")
            print(f"  Unit: {temp_data.unit}")
            print(f"  Timestamp: {temp_data.timestamp}")
            
        elif topic == "sensor/compass":
            compass_data = sensor_pb2.CompassData()
            compass_data.ParseFromString(payload)
            print(f"Compass Data:")
            print(f"  Heading: {compass_data.heading}°")
            print(f"  Unit: {compass_data.unit}")
            print(f"  Timestamp: {compass_data.timestamp}")
            
        elif topic == "sensor/gps":
            gps_data = sensor_pb2.GpsPositionData()
            gps_data.ParseFromString(payload)
            print(f"GPS Data:")
            print(f"  Latitude: {gps_data.position.latitude}")
            print(f"  Longitude: {gps_data.position.longitude}")
            print(f"  Altitude: {gps_data.position.altitude}m")
            print(f"  Accuracy: {gps_data.position.accuracy}m")
            print(f"  Unit: {gps_data.unit}")
            print(f"  Timestamp: {gps_data.timestamp}")
            
        elif topic == "sensor/status":
            status_data = sensor_pb2.StatusMessage()
            status_data.ParseFromString(payload)
            status_names = {0: "UNKNOWN", 1: "ONLINE", 2: "OFFLINE", 3: "ERROR"}
            print(f"Status Data:")
            print(f"  Status: {status_names.get(status_data.status, 'UNKNOWN')}")
            print(f"  Device ID: {status_data.device_id}")
            print(f"  Message: {status_data.message}")
            print(f"  Timestamp: {status_data.timestamp}")
            
        else:
            print(f"Unknown topic: {topic}")
            print(f"Payload length: {len(payload)} bytes")
            
    except Exception as e:
        print(f"Error parsing protobuf message: {e}")
        print(f"Raw payload length: {len(payload)} bytes")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 test_protobuf.py <mqtt_broker_host> [port]")
        print("Example: python3 test_protobuf.py localhost 1883")
        sys.exit(1)
    
    broker_host = sys.argv[1]
    broker_port = int(sys.argv[2]) if len(sys.argv) > 2 else 1883
    
    print(f"Connecting to MQTT broker: {broker_host}:{broker_port}")
    print("This script will decode protobuf messages from the sensor simulator.")
    print("Make sure the sensor simulator is running and publishing data.")
    print("Press Ctrl+C to stop.\n")
    
    # Create MQTT client
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    
    try:
        client.connect(broker_host, broker_port, 60)
        client.loop_forever()
    except KeyboardInterrupt:
        print("\nStopping protobuf test...")
        client.disconnect()
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main() 