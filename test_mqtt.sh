#!/bin/bash

# Test script for IMX8MP Sensor Simulator MQTT functionality
# This script helps test the MQTT publishing functionality

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
MQTT_BROKER="localhost"
MQTT_PORT="1883"
TEST_DURATION=30
VERBOSE=false

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_test() {
    echo -e "${BLUE}[TEST]${NC} $1"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -b, --broker HOST        MQTT broker hostname (default: localhost)"
    echo "  -p, --port PORT          MQTT broker port (default: 1883)"
    echo "  -d, --duration SECONDS   Test duration in seconds (default: 30)"
    echo "  -v, --verbose            Verbose output"
    echo "  -h, --help               Show this help message"
    echo ""
    echo "This script tests the MQTT functionality of the sensor simulator."
    echo "It will start the sensor simulator and monitor MQTT messages."
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -b|--broker)
            MQTT_BROKER="$2"
            shift 2
            ;;
        -p|--port)
            MQTT_PORT="$2"
            shift 2
            ;;
        -d|--duration)
            TEST_DURATION="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

print_status "IMX8MP Sensor Simulator MQTT Test"
print_status "MQTT Broker: $MQTT_BROKER:$MQTT_PORT"
print_status "Test Duration: $TEST_DURATION seconds"
echo ""

# Check if sensor_simulator binary exists
if [[ ! -f "./build/sensor_simulator" ]]; then
    print_error "Sensor simulator binary not found!"
    print_error "Please build the application first: ./build.sh"
    exit 1
fi

# Check if mosquitto_sub is available
if ! command -v mosquitto_sub &> /dev/null; then
    print_error "mosquitto_sub not found!"
    print_error "Please install mosquitto-clients package:"
    print_error "  Ubuntu/Debian: sudo apt install mosquitto-clients"
    print_error "  CentOS/RHEL: sudo yum install mosquitto-clients"
    exit 1
fi

# Check if mosquitto_pub is available
if ! command -v mosquitto_pub &> /dev/null; then
    print_error "mosquitto_pub not found!"
    print_error "Please install mosquitto-clients package"
    exit 1
fi

print_status "Checking MQTT broker connectivity..."

# Test MQTT broker connectivity
if ! mosquitto_pub -h "$MQTT_BROKER" -p "$MQTT_PORT" -t "test/connection" -m "test" -q 0 &>/dev/null; then
    print_error "Cannot connect to MQTT broker at $MQTT_BROKER:$MQTT_PORT"
    print_error "Please ensure the MQTT broker is running and accessible"
    print_error ""
    print_error "To start a local MQTT broker:"
    print_error "  Ubuntu/Debian: sudo systemctl start mosquitto"
    print_error "  CentOS/RHEL: sudo systemctl start mosquitto"
    exit 1
fi

print_status "MQTT broker is accessible"
echo ""

# Start sensor simulator in background
print_test "Starting sensor simulator..."
./build/sensor_simulator --broker "$MQTT_BROKER:$MQTT_PORT" --interval 1000 &
SIMULATOR_PID=$!

# Wait a moment for the simulator to start
sleep 2

# Check if simulator is running
if ! kill -0 $SIMULATOR_PID 2>/dev/null; then
    print_error "Failed to start sensor simulator"
    exit 1
fi

print_status "Sensor simulator started (PID: $SIMULATOR_PID)"
echo ""

# Start MQTT subscriber in background
print_test "Starting MQTT subscriber..."
if [[ "$VERBOSE" == true ]]; then
    mosquitto_sub -h "$MQTT_BROKER" -p "$MQTT_PORT" -t "sensor/#" -v &
else
    mosquitto_sub -h "$MQTT_BROKER" -p "$MQTT_PORT" -t "sensor/#" -v | head -20 &
fi
SUBSCRIBER_PID=$!

# Wait for test duration
print_status "Running test for $TEST_DURATION seconds..."
print_status "Press Ctrl+C to stop early"
echo ""

# Monitor for messages
MESSAGE_COUNT=0
START_TIME=$(date +%s)

while true; do
    CURRENT_TIME=$(date +%s)
    ELAPSED=$((CURRENT_TIME - START_TIME))
    
    if [[ $ELAPSED -ge $TEST_DURATION ]]; then
        break
    fi
    
    # Check if simulator is still running
    if ! kill -0 $SIMULATOR_PID 2>/dev/null; then
        print_error "Sensor simulator stopped unexpectedly"
        break
    fi
    
    sleep 1
done

# Stop the simulator
print_status "Stopping sensor simulator..."
kill $SIMULATOR_PID 2>/dev/null || true

# Stop the subscriber
print_status "Stopping MQTT subscriber..."
kill $SUBSCRIBER_PID 2>/dev/null || true

# Wait for processes to terminate
wait $SIMULATOR_PID 2>/dev/null || true
wait $SUBSCRIBER_PID 2>/dev/null || true

echo ""
print_status "Test completed!"
print_status "Check the output above for MQTT messages"
echo ""

# Test individual topics
print_test "Testing individual MQTT topics..."

print_status "Testing temperature topic..."
mosquitto_sub -h "$MQTT_BROKER" -p "$MQTT_PORT" -t "sensor/temperature" -C 1 -W 5

print_status "Testing compass topic..."
mosquitto_sub -h "$MQTT_BROKER" -p "$MQTT_PORT" -t "sensor/compass" -C 1 -W 5

print_status "Testing GPS topic..."
mosquitto_sub -h "$MQTT_BROKER" -p "$MQTT_PORT" -t "sensor/gps" -C 1 -W 5

print_status "Testing combined data topic..."
mosquitto_sub -h "$MQTT_BROKER" -p "$MQTT_PORT" -t "sensor/all" -C 1 -W 5

echo ""
print_status "MQTT test completed successfully!"
print_status "If you received JSON messages above, the MQTT functionality is working correctly." 