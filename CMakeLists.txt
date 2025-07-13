cmake_minimum_required(VERSION 3.16)
project(sensor_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

# Include directories
include_directories(${MOSQUITTO_INCLUDE_DIRS})

# Add executable
add_executable(sensor_simulator 
    src/main.cpp
    src/sensor_simulator.cpp
    src/mqtt_client.cpp
    src/data_converter.cpp
)

# Link libraries
target_link_libraries(sensor_simulator ${MOSQUITTO_LIBRARIES})

# Compiler flags
target_compile_options(sensor_simulator PRIVATE ${MOSQUITTO_CFLAGS_OTHER})

# Install target
install(TARGETS sensor_simulator DESTINATION bin) 