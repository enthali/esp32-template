# Distance Measurement with LED Strip Display

A project that uses an ESP32 to measure distance with an ultrasonic sensor and displays the measurement on a WS2812 LED strip with a web interface.

## Project Overview

This project combines hardware sensing and visual feedback to create an interactive distance measurement system. The measured distance is displayed both visually on an LED strip and remotely via a web interface.

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
| WS2812 LED Strip | Data | GPIO13 |
| HC-SR04 Trigger | Trigger | GPIO14 |
| HC-SR04 Echo | Echo | GPIO15 |
| Power | VCC | 5V/3.3V |
| Ground | GND | GND |

## Project Objectives

1. **LED Strip Control**: Implement addressable LED control for visual feedback
2. **Distance Measurement**: Integrate ultrasonic sensor for accurate distance readings
3. **Visual Indication**: Map distance measurements to LED positions on the strip
4. **Web Interface**: Provide remote monitoring via HTTP server

## Implementation Plan

### Step 1: LED Strip Animation ✅
- Configure WS2812 LED strip with RMT backend
- Implement basic LED animations (running light effect)
- Test all 40 LEDs for proper functionality
- Verify power requirements and stability

### Step 2: Ultrasonic Sensor Integration
- Connect HC-SR04 sensor to designated GPIO pins
- Implement distance measurement algorithm
- Output readings to serial console
- Calibrate and validate sensor accuracy

### Step 3: Distance-to-LED Mapping
- Create mapping algorithm (distance → LED position)
- Integrate sensor readings with LED display
- Implement smooth transitions and visual effects
- Test with various distances and validate accuracy

### Step 4: Web Interface
- Set up ESP32 as WiFi access point or station
- Implement HTTP server for web interface
- Create responsive webpage displaying distance
- Add real-time updates and configuration options

## Technical Specifications

- **LED Strip**: 40 x WS2812 individually addressable LEDs
- **Distance Range**: 2cm - 400cm (HC-SR04 specification)
- **Update Rate**: ~10Hz for smooth visual feedback
- **Communication**: WiFi for web interface
- **Power**: USB or external 5V supply

## Development Environment

- **Framework**: ESP-IDF v5.4.1
- **Language**: C
- **IDE**: Visual Studio Code with ESP-IDF extension
- **Target**: ESP32

## Project Structure

```
├── CMakeLists.txt
├── components/                   # Hardware abstraction components
│   └── led_controller/          # WS2812 LED strip hardware interface
│       ├── CMakeLists.txt
│       ├── include/led_controller.h
│       └── led_controller.c
├── main/                        # Application logic
│   ├── CMakeLists.txt
│   ├── main.c                   # Main application entry point
│   ├── distance_sensor.h/c      # HC-SR04 ultrasonic sensor (future)
│   ├── display_logic.h/c        # Distance-to-LED mapping logic (future)
│   ├── web_server.h/c           # HTTP server for web interface (future)
│   └── test/                    # Application tests
├── ARCHITECTURE.md              # Detailed technical architecture
├── .gitignore                   # Git ignore patterns
└── README.md                    # This file
```

## Build and Flash

1. Set the correct target:
   ```
   idf.py set-target esp32
   ```

2. Configure the project:
   ```
   idf.py menuconfig
   ```

3. Build and flash:
   ```
   idf.py build flash monitor
   ```

## Usage

1. Power on the ESP32 with connected hardware
2. Observe LED strip animation during startup
3. Place objects at various distances from the sensor
4. Watch the LED position change based on distance
5. Connect to the web interface for remote monitoring

## Testing Strategy

- **Unit Testing**: Test each component individually
- **Integration Testing**: Verify component interactions
- **Performance Testing**: Validate update rates and accuracy
- **User Testing**: Ensure intuitive visual feedback

## Future Enhancements

- Multiple distance sensors for 2D/3D mapping
- Color-coded distance ranges
- Data logging and historical analysis
- Mobile app integration
- Voice feedback for accessibility

## Troubleshooting

- **LED not working**: Check GPIO13 connection and power supply
- **Sensor not responding**: Verify GPIO14/15 connections and timing
- **Web interface unavailable**: Check WiFi configuration and IP address
- **Inconsistent readings**: Calibrate sensor and check for interference

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is open source and available under the MIT License.

## Author

Created as a learning project for ESP32 development with ESP-IDF.
