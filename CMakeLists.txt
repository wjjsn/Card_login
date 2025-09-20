cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME PN532_test)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")
include("CherryUSB/cherryusb.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)
add_subdirectory(EmLib)
add_subdirectory(CherryUSB)
# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    "Core/Src/task.cpp"
    "Core/Src/hid_keyboard_template.c"
    "${cherryusb_srcs}"
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${cherryusb_incs}
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# 创建自动配置的 launch.json
file(WRITE ${CMAKE_SOURCE_DIR}/.vscode/launch.json.in
[=[
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug",
            "cwd": "${workspaceFolder}",
            "executable": "@PROGRAM_PATH@",
            "request": "launch",
            "type": "cortex-debug",
            "runToEntryPoint": "main",
            "servertype": "jlink",
            "device": "@device@",
            "liveWatch": {
                "enabled": true,
                "samplesPerSecond": 4
            },
            "swoConfig": {
                "enabled": true,
                "source": "probe",
                "swoFrequency": 2000000,
                "cpuFrequency": 48000000,
                "decoders": [
                    {
                        "port": 0,
                        "type": "console",
                        "label": "SWO output",
                        "encoding": "utf8"
                    }
                ]
            },
            "preLaunchTask": "build",
        },
        {
            "name": "Run",
            "cwd": "${workspaceFolder}",
            "executable": "@PROGRAM_PATH@",
            "request": "launch",
            "type": "cortex-debug",
            "runToEntryPoint": "",
            "servertype": "jlink",
            "device": "@device@",
            "liveWatch": {
                "enabled": true,
                "samplesPerSecond": 4
            },
            "rttConfig": {
                "enabled": true,
                "address": "auto",
                "decoders": [
                    {
                        "label": "",
                        "port": 0,
                        "type": "console"
                    }
                ]
            },
            "preLaunchTask": "build",
        }
    ]
}
]=])

# 替换占位符
set(PROGRAM_PATH ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.elf)
set(device "STM32F103xb")

configure_file(
    ${CMAKE_SOURCE_DIR}/.vscode/launch.json.in
    ${CMAKE_SOURCE_DIR}/.vscode/launch.json
    @ONLY
)