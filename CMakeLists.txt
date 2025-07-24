cmake_minimum_required(VERSION 3.16)
project(sensor_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)
find_package(Protobuf REQUIRED)

# Include directories
include_directories(${MOSQUITTO_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Generate protobuf files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS proto/sensor.proto proto/actions.proto proto/fileops.proto)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Add debug flags for Debug builds
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Add executable
add_executable(sensor_simulator 
    src/main.cpp
    src/sensor_simulator.cpp
    src/mqtt_client.cpp
    src/protobuf_converter.cpp
    ${PROTO_SRCS}
)

# Link libraries
target_link_libraries(sensor_simulator ${MOSQUITTO_LIBRARIES} protobuf)

# Compiler flags
target_compile_options(sensor_simulator PRIVATE ${MOSQUITTO_CFLAGS_OTHER})

# Install target
install(TARGETS sensor_simulator DESTINATION bin) 