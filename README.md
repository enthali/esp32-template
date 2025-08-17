# Distance Measurement with LED Strip 
## Quick Overview

- ✅ **Hardware Integration**: LED strip control and distance sensor measurement
- ✅ **Real-time Visualization**: Distance-to-LED mapping with error indicators
- ✅ **WiFi Connectivity**: Smart connection with captive portal fallback
- 🔄 **Web Interface**: Basic server implemented, static UI in progress

## Technical Specificationsrview

The project uses an ESP32 to measure distance with an ultrasonic sensor HR-SR04 and displays the measurement on a WS2812 LED strip and a web interface.

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

## Implementation Status

For detailed implementation progress, technical specifications, and step-by-step tracking:

- **Implementation Plan**: See [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md)
- **Technical Architecture**: See [ARCHITECTURE.md](ARCHITECTURE.md)

### Quick Status Overview

- ✅ **Step 1**: LED Strip Animation - **COMPLETED**
- ✅ **Step 2**: Ultrasonic Sensor Integration - **COMPLETED**
- ✅ **Step 3**: Distance-to-LED Mapping - **COMPLETED**
- � **Step 4**: Web Interface - **IN PROGRESS** (Step 4.1 WiFi/Captive Portal completed)

## Technical Specifications

- **LED Strip**: 40 x WS2812 individually addressable LEDs
- **Sensor Range**: 2cm - 400cm (HC-SR04 specification)
- **Update Rate**: up to 20Hz for real-time visual feedback (50ms measurement interval)
- **Communication**: WiFi with smart AP/STA switching and captive portal
- **Web Interface**: HTTP server with mobile-responsive design
- **Power**: USB or external 5V supply

## Development Environment

- **Framework**: ESP-IDF v5.4.1
- **Language**: C
- **IDE**: Visual Studio Code with ESP-IDF extension
- **Target**: ESP32

## Project Structure

```
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

## Current System Usage

### Current Distance Measurement Mode

1. Power on the ESP32 with connected hardware (LED strip and HC-SR04 sensor)
2. The system performs a one-time LED hardware test at startup
3. **WiFi Connection**:
   - If no WiFi credentials stored: Creates "ESP32-Distance-Sensor" AP with captive portal
   - If credentials stored: Connects to configured WiFi network
   - Fallback to AP mode if connection fails
4. **Real-time Operation**:
   - Distance measurements every 100ms (10Hz update rate)
   - LED strip displays distance: 10cm-50cm mapped to LEDs 0-39
   - Green LEDs for normal range, red LEDs for out of Range measurement indicaton
   
5. **Web Interface**: Access in AP moder at 192.168.4.1 for status and configuration
6. Monitor serial output for detailed system logging

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is open source and available under the MIT License.

## Author

Created as a learning and demo project for ESP32 development with ESP-IDF utilizing FreeRTOS, OpenFastTrack for requitement and Design Dokumentation. 
