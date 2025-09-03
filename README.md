# ESP32 Distance Sensor with LED Display

An ESP32-based IoT device that measures distance using an ultrasonic sensor and displays the results on an addressable LED strip. The system includes WiFi connectivity and a web interface for configuration and monitoring.

## Features

- **Real-time Distance Measurement**: HC-SR04 ultrasonic sensor with millimeter precision
- **Visual Display**: WS2812 LED strip shows distance as position and color
- **WiFi Connectivity**: Automatic network connection with captive portal setup
- **Web Interface**: Mobile-responsive configuration and status interface
- **Embedded Optimization**: Integer-only arithmetic for optimal ESP32 performance

## Hardware Requirements

- ESP32 development board (ESP32-WROOM-32 or compatible)
- 40x WS2812 LED strip
- HC-SR04 ultrasonic sensor
- USB cable for power and programming
- Jumper wires for connections
- External 5V power supply (if needed for LED strip)

## Pin Configuration

| Component | Pin | GPIO |
|-----------|-----|------|
| WS2812 LED Strip | Data | GPIO12 |
| HC-SR04 Trigger | Trigger | GPIO14 |
| HC-SR04 Echo | Echo | GPIO13 |
| Power | VCC | 5V/3.3V |
| Ground | GND | GND |

## How It Works

1. **Power On**: ESP32 starts with hardware initialization and LED test sequence
2. **WiFi Setup**:
   - Connects to stored WiFi credentials automatically
   - Creates "ESP32-Distance-Sensor" access point if no credentials stored
   - Captive portal guides through network setup
3. **Distance Measurement**: Continuous HC-SR04 sensor readings every 100ms
4. **Visual Display**: LED strip shows distance as:
   - **Green LED**: Normal range (10cm-50cm mapped to LEDs 0-39)
   - **Red LED**: Out of range (below 10cm or above 50cm)
5. **Web Interface**: Access at device IP for configuration and monitoring

## Technical Specifications

- **LED Strip**: 40 x WS2812 individually addressable LEDs
- **Sensor Range**: 2cm - 400cm (HC-SR04 specification)  
- **Update Rate**: 10Hz real-time visual feedback
- **Communication**: WiFi with smart AP/STA switching and captive portal
- **Web Interface**: HTTP server with mobile-responsive design
- **Power**: USB or external 5V supply
- **Architecture**: Integer-only arithmetic optimized for ESP32 performance

## Development Environment

- **Framework**: ESP-IDF v5.4.1
- **Language**: C
- **IDE**: Visual Studio Code with ESP-IDF extension
- **Target**: ESP32
- **Container Support**: Docker Dev Container and GitHub Codespaces ready

## Development Options

### Local Development
- **Windows**: Native ESP-IDF v5.4.1 installation
- **Build**: `cmd /c "cd /D ... && idf.py build"`
- **Hardware**: Direct USB connection for flashing

### Containerized Development  
- **Docker**: VS Code Dev Container with ESP-IDF pre-installed
- **Codespaces**: GitHub Codespaces with web-based development
- **Cross-platform**: Consistent environment on any OS

See [`.devcontainer/README.md`](.devcontainer/README.md) for detailed container setup instructions.

## Project Structure

```text
main/
├── main.c                           # ESP-IDF required entry point
├── config.h                         # Global configuration  
├── CMakeLists.txt                   # Main component definition
└── components/                      # All custom components
    ├── cert_handler/                # SSL certificate management
    ├── config_manager/              # Configuration management
    ├── display_logic/               # LED display control
    ├── distance_sensor/             # HC-SR04 sensor interface
    ├── led_controller/              # WS2812 LED strip control
    ├── startup_tests/               # Hardware validation tests
    └── web_server/                  # HTTP server + WiFi manager
        ├── wifi_manager.c           # WiFi management (merged here)
        ├── wifi_manager.h
        ├── web_server.c
        ├── web_server.h
        └── www/                     # Web assets
```

## Build and Flash

1. Set the correct target:

   ```bash
   idf.py set-target esp32
   ```

2. Configure the project (optional):

   ```bash
   idf.py menuconfig
   ```

3. Build, flash, and monitor:

   ```bash
   idf.py build flash monitor
   ```

For detailed build instructions including Windows-specific commands, see [.github/BUILD_INSTRUCTIONS.md](.github/BUILD_INSTRUCTIONS.md).

## License

This project is open source and available under the MIT License.

## Author

Created as a learning and demo project for embedded ESP32 development with ESP-IDF, utilizing FreeRTOS, OpenFastTrack for requirements and design documentation, and GitHub Copilot's latest features.

