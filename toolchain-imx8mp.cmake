# CMake Toolchain File for IMX8MP Cross-compilation
# This file configures CMake for cross-compilation targeting ARM64 architecture

# Target system
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the cross compiler
# Adjust these paths according to your toolchain installation
set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_ASM_COMPILER aarch64-linux-gnu-gcc)

# Where is the target environment
# Adjust these paths according to your sysroot location
set(CMAKE_FIND_ROOT_PATH /opt/fsl-imx-xwayland/5.15-kirkstone/sysroots/aarch64-poky-linux)

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a -mtune=cortex-a72")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a -mtune=cortex-a72")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath-link,${CMAKE_FIND_ROOT_PATH}/lib")

# Set pkg-config to use the target sysroot
set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_FIND_ROOT_PATH})
set(ENV{PKG_CONFIG_PATH} "${CMAKE_FIND_ROOT_PATH}/usr/lib/pkgconfig:${CMAKE_FIND_ROOT_PATH}/usr/share/pkgconfig")

# Set up environment for finding libraries
set(CMAKE_LIBRARY_PATH ${CMAKE_FIND_ROOT_PATH}/lib ${CMAKE_FIND_ROOT_PATH}/usr/lib)
set(CMAKE_INCLUDE_PATH ${CMAKE_FIND_ROOT_PATH}/usr/include)

# Additional compiler definitions for IMX8MP
add_definitions(-DIMX8MP)
add_definitions(-D__ARM_ARCH_8A__)

# Set RPATH settings
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_FIND_ROOT_PATH}/lib")

# Optional: Set specific paths for mosquitto if needed
# set(MOSQUITTO_INCLUDE_DIRS "${CMAKE_FIND_ROOT_PATH}/usr/include")
# set(MOSQUITTO_LIBRARIES "${CMAKE_FIND_ROOT_PATH}/usr/lib/libmosquitto.so")

# Print configuration information
message(STATUS "Cross-compiling for IMX8MP (ARM64)")
message(STATUS "Target system: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Target processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Root path: ${CMAKE_FIND_ROOT_PATH}") 