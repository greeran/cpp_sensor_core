#!/bin/bash

# IMX8MP Sensor Simulator Deployment Script
# This script helps deploy the sensor simulator to the target device

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
TARGET_HOST=""
TARGET_USER="root"
TARGET_PORT="22"
INSTALL_PATH="/usr/local/bin"
SERVICE_NAME="sensor-simulator"
BUILD_TYPE="native"
CLEAN_BUILD=false
INSTALL_SERVICE=true
START_SERVICE=false

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

print_deploy() {
    echo -e "${BLUE}[DEPLOY]${NC} $1"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --host HOST          Target hostname or IP address (required)"
    echo "  -u, --user USER          Target username (default: root)"
    echo "  -p, --port PORT          SSH port (default: 22)"
    echo "  -i, --install-path PATH  Installation path (default: /usr/local/bin)"
    echo "  -s, --service-name NAME  Service name (default: sensor-simulator)"
    echo "  -t, --build-type TYPE    Build type: native, cross (default: native)"
    echo "  -c, --clean              Clean build before deployment"
    echo "  --no-service             Skip service installation"
    echo "  --start-service          Start service after installation"
    echo "  -v, --verbose            Verbose output"
    echo "  --help                   Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 -h 192.168.1.100                    # Deploy to specific IP"
    echo "  $0 -h imx8mp.local -u admin            # Deploy with custom user"
    echo "  $0 -h 192.168.1.100 -c --start-service # Clean build and start service"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--host)
            TARGET_HOST="$2"
            shift 2
            ;;
        -u|--user)
            TARGET_USER="$2"
            shift 2
            ;;
        -p|--port)
            TARGET_PORT="$2"
            shift 2
            ;;
        -i|--install-path)
            INSTALL_PATH="$2"
            shift 2
            ;;
        -s|--service-name)
            SERVICE_NAME="$2"
            shift 2
            ;;
        -t|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        --no-service)
            INSTALL_SERVICE=false
            shift
            ;;
        --start-service)
            START_SERVICE=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --help)
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

# Validate required parameters
if [[ -z "$TARGET_HOST" ]]; then
    print_error "Target host is required!"
    print_error "Use -h or --host to specify the target hostname or IP address"
    show_usage
    exit 1
fi

# Validate build type
if [[ "$BUILD_TYPE" != "native" && "$BUILD_TYPE" != "cross" ]]; then
    print_error "Invalid build type: $BUILD_TYPE"
    print_error "Valid types: native, cross"
    exit 1
fi

print_status "IMX8MP Sensor Simulator Deployment"
print_status "Target: $TARGET_USER@$TARGET_HOST:$TARGET_PORT"
print_status "Install path: $INSTALL_PATH"
print_status "Build type: $BUILD_TYPE"
print_status "Install service: $INSTALL_SERVICE"
print_status "Start service: $START_SERVICE"
echo ""

# Check if binary exists or needs to be built
if [[ ! -f "./build/sensor_simulator" ]] || [[ "$CLEAN_BUILD" == true ]]; then
    print_deploy "Building sensor simulator..."
    
    BUILD_ARGS=""
    if [[ "$CLEAN_BUILD" == true ]]; then
        BUILD_ARGS="$BUILD_ARGS -c"
    fi
    
    if [[ "$BUILD_TYPE" == "cross" ]]; then
        BUILD_ARGS="$BUILD_ARGS -t cross"
    fi
    
    if [[ "$VERBOSE" == true ]]; then
        BUILD_ARGS="$BUILD_ARGS -v"
    fi
    
    ./build.sh $BUILD_ARGS
    
    if [[ ! -f "./build/sensor_simulator" ]]; then
        print_error "Build failed! Binary not found."
        exit 1
    fi
else
    print_status "Using existing binary: ./build/sensor_simulator"
fi

# Test SSH connectivity
print_deploy "Testing SSH connectivity..."
if ! ssh -o ConnectTimeout=10 -o BatchMode=yes -p "$TARGET_PORT" "$TARGET_USER@$TARGET_HOST" "echo 'SSH connection successful'" &>/dev/null; then
    print_error "Cannot connect to target via SSH!"
    print_error "Please ensure:"
    print_error "  1. SSH is enabled on the target"
    print_error "  2. SSH key authentication is set up"
    print_error "  3. Target hostname/IP is correct"
    print_error "  4. Target user has appropriate permissions"
    exit 1
fi

print_status "SSH connection successful"
echo ""

# Create remote directories
print_deploy "Creating remote directories..."
ssh -p "$TARGET_PORT" "$TARGET_USER@$TARGET_HOST" "mkdir -p $INSTALL_PATH"

# Copy binary to target
print_deploy "Copying binary to target..."
scp -P "$TARGET_PORT" "./build/sensor_simulator" "$TARGET_USER@$TARGET_HOST:$INSTALL_PATH/"

# Make binary executable
print_deploy "Setting executable permissions..."
ssh -p "$TARGET_PORT" "$TARGET_USER@$TARGET_HOST" "chmod +x $INSTALL_PATH/sensor_simulator"

# Verify installation
print_deploy "Verifying installation..."
if ! ssh -p "$TARGET_PORT" "$TARGET_USER@$TARGET_HOST" "test -x $INSTALL_PATH/sensor_simulator"; then
    print_error "Installation verification failed!"
    exit 1
fi

print_status "Binary installed successfully"
echo ""

# Install systemd service if requested
if [[ "$INSTALL_SERVICE" == true ]]; then
    print_deploy "Installing systemd service..."
    
    # Copy service file to target
    scp -P "$TARGET_PORT" "./sensor-simulator.service" "$TARGET_USER@$TARGET_HOST:/tmp/"
    
    # Install service file
    ssh -p "$TARGET_PORT" "$TARGET_USER@$TARGET_HOST" "cp /tmp/sensor-simulator.service /etc/systemd/system/$SERVICE_NAME.service"
    
    # Reload systemd
    ssh -p "$TARGET_PORT" "$TARGET_USER@$TARGET_HOST" "systemctl daemon-reload"
    
    # Enable service
    ssh -p "$TARGET_PORT" "$TARGET_USER@$TARGET_HOST" "systemctl enable $SERVICE_NAME"
    
    print_status "Systemd service installed and enabled"
    echo ""
fi

# Start service if requested
if [[ "$START_SERVICE" == true ]]; then
    print_deploy "Starting sensor simulator service..."
    ssh -p "$TARGET_PORT" "$TARGET_USER@$TARGET_HOST" "systemctl start $SERVICE_NAME"
    
    # Check service status
    sleep 2
    SERVICE_STATUS=$(ssh -p "$TARGET_PORT" "$TARGET_USER@$TARGET_HOST" "systemctl is-active $SERVICE_NAME")
    
    if [[ "$SERVICE_STATUS" == "active" ]]; then
        print_status "Service started successfully"
    else
        print_warning "Service may not be running. Check status with: systemctl status $SERVICE_NAME"
    fi
    echo ""
fi

# Show deployment summary
print_status "Deployment completed successfully!"
echo ""
print_status "Deployment Summary:"
print_status "  Binary: $INSTALL_PATH/sensor_simulator"
if [[ "$INSTALL_SERVICE" == true ]]; then
    print_status "  Service: $SERVICE_NAME"
    print_status "  Service file: /etc/systemd/system/$SERVICE_NAME.service"
fi
echo ""
print_status "Useful commands:"
print_status "  Check service status: ssh $TARGET_USER@$TARGET_HOST 'systemctl status $SERVICE_NAME'"
print_status "  View logs: ssh $TARGET_USER@$TARGET_HOST 'journalctl -u $SERVICE_NAME -f'"
print_status "  Start service: ssh $TARGET_USER@$TARGET_HOST 'systemctl start $SERVICE_NAME'"
print_status "  Stop service: ssh $TARGET_USER@$TARGET_HOST 'systemctl stop $SERVICE_NAME'"
print_status "  Restart service: ssh $TARGET_USER@$TARGET_HOST 'systemctl restart $SERVICE_NAME'"
echo ""
print_status "MQTT Topics to monitor:"
print_status "  sensor/temperature - CPU temperature data"
print_status "  sensor/compass - Compass heading data"
print_status "  sensor/gps - GPS position data"
print_status "  sensor/all - Combined sensor data"
print_status "  sensor/status - Online/offline status" 