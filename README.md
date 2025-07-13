# IMX8MP Sensor Simulator

A C++ application for the IMX8MP that simulates sensor data (CPU temperature, compass, and GPS) and publishes it to MQTT topics.

## Features

- **CPU Temperature Simulation**: Realistic temperature variations with configurable ranges
- **Compass Heading Simulation**: Gradual heading changes with configurable variation
- **GPS Position Simulation**: Position drift with realistic movement patterns
- **MQTT Publishing**: Publishes sensor data to multiple MQTT topics
- **JSON Format**: All data is published in structured JSON format
- **Configurable**: Command-line options for all simulation parameters
- **Graceful Shutdown**: Proper signal handling and cleanup

## MQTT Topics

The application publishes to the following MQTT topics:

- `sensor/temperature` - CPU temperature data
- `sensor/compass` - Compass heading data  
- `sensor/gps` - GPS position data
- `sensor/all` - Combined sensor data
- `sensor/status` - Online/offline status (retained)

## JSON Data Format

### Temperature Data
```json
{
  "timestamp": "2024-01-15T10:30:45.123Z",
  "temperature": 65.42,
  "unit": "celsius"
}
```

### Compass Data
```json
{
  "timestamp": "2024-01-15T10:30:45.123Z",
  "heading": 245.67,
  "unit": "degrees"
}
```

### GPS Data
```json
{
  "timestamp": "2024-01-15T10:30:45.123Z",
  "latitude": 37.774900,
  "longitude": -122.419400,
  "altitude": 100.50,
  "unit": "decimal_degrees"
}
```

### Combined Data
```json
{
  "timestamp": "2024-01-15T10:30:45.123Z",
  "cpu_temperature": 65.42,
  "compass_heading": 245.67,
  "gps": {
    "latitude": 37.774900,
    "longitude": -122.419400,
    "altitude": 100.50
  }
}
```

## Prerequisites

### For IMX8MP (Cross-compilation)
- Yocto Project or similar embedded Linux build system
- Cross-compilation toolchain for ARM64
- libmosquitto development libraries

### For Development (Native compilation)
- GCC 7+ or Clang 6+
- CMake 3.16+
- libmosquitto development libraries

## Installation

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake libmosquitto-dev pkg-config
```

### CentOS/RHEL
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake3 libmosquitto-devel pkg-config
```

## Building

### Native Build
```bash
mkdir build
cd build
cmake ..
make
```

### Cross-compilation for IMX8MP
```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-imx8mp.cmake ..
make
```

## Usage

### Basic Usage
```bash
./sensor_simulator
```

### Advanced Usage
```bash
./sensor_simulator \
  --broker mqtt.example.com:1883 \
  --interval 2000 \
  --temp-min 40.0 \
  --temp-max 80.0 \
  --compass-var 10.0 \
  --gps-drift 0.5 \
  --client-id my_imx8mp_sensor
```

### Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `-b, --broker HOST[:PORT]` | MQTT broker address | localhost:1883 |
| `-i, --interval MS` | Update interval in milliseconds | 1000 |
| `-t, --temp-min TEMP` | Minimum CPU temperature (°C) | 35.0 |
| `-T, --temp-max TEMP` | Maximum CPU temperature (°C) | 85.0 |
| `-c, --compass-var DEG` | Compass variation (degrees) | 5.0 |
| `-g, --gps-drift MPS` | GPS drift (m/s) | 0.1 |
| `-u, --username USER` | MQTT username | (none) |
| `-p, --password PASS` | MQTT password | (none) |
| `-d, --client-id ID` | MQTT client ID | sensor_simulator |
| `-h, --help` | Show help message | |

## Deployment on IMX8MP

### 1. Build the Application
```bash
# Set up your Yocto environment
source oe-init-build-env

# Build the application
bitbake sensor-simulator
```

### 2. Install on Target
```bash
# Copy the binary to your IMX8MP
scp sensor_simulator root@imx8mp-ip:/usr/local/bin/

# Make executable
chmod +x /usr/local/bin/sensor_simulator
```

### 3. Create Systemd Service
Create `/etc/systemd/system/sensor-simulator.service`:
```ini
[Unit]
Description=IMX8MP Sensor Simulator
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/sensor_simulator --broker mqtt-broker:1883
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

### 4. Enable and Start Service
```bash
systemctl daemon-reload
systemctl enable sensor-simulator
systemctl start sensor-simulator
```

## Testing

### Using mosquitto_sub
```bash
# Subscribe to all sensor topics
mosquitto_sub -h localhost -t "sensor/#" -v

# Subscribe to specific topic
mosquitto_sub -h localhost -t "sensor/temperature" -v
```

### Using MQTT Explorer
1. Connect to your MQTT broker
2. Subscribe to `sensor/#`
3. Monitor the published data in real-time

## Configuration Examples

### High-frequency Updates
```bash
./sensor_simulator --interval 500
```

### Realistic Temperature Range
```bash
./sensor_simulator --temp-min 45.0 --temp-max 75.0
```

### Slow GPS Movement
```bash
./sensor_simulator --gps-drift 0.05
```

### Custom MQTT Broker
```bash
./sensor_simulator --broker 192.168.1.100:1883 --username user --password pass
```

## Troubleshooting

### Connection Issues
- Verify MQTT broker is running and accessible
- Check firewall settings
- Ensure correct broker address and port

### Build Issues
- Install required development packages
- Verify CMake version (3.16+)
- Check libmosquitto installation

### Runtime Issues
- Check MQTT broker connectivity
- Verify topic permissions
- Monitor system resources

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## Support

For issues and questions:
1. Check the troubleshooting section
2. Review the MQTT broker logs
3. Enable debug logging if available
4. Create an issue with detailed information 