# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if(EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(CC1101 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Run python scripts
message("Running create_font python script")
execute_process(COMMAND
    py src/python/create_font.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# Add executable. Default name is the project name, version 0.1
add_executable(${PROJECT_NAME}
        src/main.cpp
        src/graphics/graphics.cpp
        src/graphics/ST7789/ST7789.cpp
        src/matrix/matrix.cpp
        src/Socket/CC1101/CC1101.cpp
        src/Socket/Socket.cpp
        )

# Add the standard library to the build
target_link_libraries(${PROJECT_NAME}
    pico_stdlib)

# Add the standard include files to the build
target_include_directories(${PROJECT_NAME} PRIVATE
    # ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
)

# Add any user requested libraries
target_link_libraries(${PROJECT_NAME}
    hardware_spi
    hardware_i2c
    hardware_dma
    hardware_pio
    pico_multicore
)


# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)


pico_add_extra_outputs(${PROJECT_NAME})
