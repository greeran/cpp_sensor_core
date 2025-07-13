#!/bin/bash

# IMX8MP Sensor Simulator Build Script
# Supports both native and cross-compilation builds

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="native"
BUILD_DIR="build"
CLEAN_BUILD=false
INSTALL=false
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

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -t, --type TYPE        Build type: native, cross (default: native)"
    echo "  -d, --dir DIR          Build directory (default: build)"
    echo "  -c, --clean            Clean build directory before building"
    echo "  -i, --install          Install after building"
    echo "  -v, --verbose          Verbose output"
    echo "  -h, --help             Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                     # Native build"
    echo "  $0 -t cross           # Cross-compilation for IMX8MP"
    echo "  $0 -c -v              # Clean build with verbose output"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -d|--dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
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

# Validate build type
if [[ "$BUILD_TYPE" != "native" && "$BUILD_TYPE" != "cross" ]]; then
    print_error "Invalid build type: $BUILD_TYPE"
    print_error "Valid types: native, cross"
    exit 1
fi

print_status "IMX8MP Sensor Simulator Build Script"
print_status "Build type: $BUILD_TYPE"
print_status "Build directory: $BUILD_DIR"

# Check prerequisites
print_status "Checking prerequisites..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake not found. Please install CMake 3.16 or later."
    exit 1
fi

# Check CMake version
CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
CMAKE_MAJOR=$(echo $CMAKE_VERSION | cut -d. -f1)
CMAKE_MINOR=$(echo $CMAKE_VERSION | cut -d. -f2)

if [[ "$CMAKE_MAJOR" -lt 3 ]] || [[ "$CMAKE_MAJOR" -eq 3 && "$CMAKE_MINOR" -lt 16 ]]; then
    print_error "CMake version $CMAKE_VERSION is too old. Please install CMake 3.16 or later."
    exit 1
fi

print_status "CMake version: $CMAKE_VERSION"

# Check for pkg-config
if ! command -v pkg-config &> /dev/null; then
    print_error "pkg-config not found. Please install pkg-config."
    exit 1
fi

# Check for libmosquitto
if ! pkg-config --exists libmosquitto; then
    print_error "libmosquitto development libraries not found."
    print_error "Please install libmosquitto-dev (Ubuntu/Debian) or libmosquitto-devel (CentOS/RHEL)"
    exit 1
fi

MOSQUITTO_VERSION=$(pkg-config --modversion libmosquitto)
print_status "libmosquitto version: $MOSQUITTO_VERSION"

# Clean build directory if requested
if [[ "$CLEAN_BUILD" == true ]]; then
    print_status "Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
print_status "Creating build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure build
print_status "Configuring build..."

CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release"

if [[ "$BUILD_TYPE" == "cross" ]]; then
    if [[ -f "../toolchain-imx8mp.cmake" ]]; then
        CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=../toolchain-imx8mp.cmake"
        print_status "Using cross-compilation toolchain"
    else
        print_warning "Cross-compilation toolchain file not found: toolchain-imx8mp.cmake"
        print_warning "Falling back to native build"
        BUILD_TYPE="native"
    fi
fi

if [[ "$VERBOSE" == true ]]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_VERBOSE_MAKEFILE=ON"
fi

print_status "Running: cmake .. $CMAKE_ARGS"
cmake .. $CMAKE_ARGS

# Build
print_status "Building sensor simulator..."
if [[ "$VERBOSE" == true ]]; then
    make VERBOSE=1
else
    make
fi

# Check if build was successful
if [[ -f "sensor_simulator" ]]; then
    print_status "Build successful!"
    
    # Show binary information
    if command -v file &> /dev/null; then
        print_status "Binary information:"
        file sensor_simulator
    fi
    
    if command -v ldd &> /dev/null; then
        print_status "Dependencies:"
        ldd sensor_simulator 2>/dev/null || echo "Static binary or dependencies not available"
    fi
    
    # Install if requested
    if [[ "$INSTALL" == true ]]; then
        print_status "Installing sensor simulator..."
        sudo make install
        print_status "Installation complete!"
    fi
    
    print_status "Build completed successfully!"
    print_status "You can run the application with: ./sensor_simulator"
    
else
    print_error "Build failed! Binary not found."
    exit 1
fi 