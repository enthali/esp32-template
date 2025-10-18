# ESP32 Template Emulator Support

This document explains the QEMU emulator support in the ESP32 template, which enables development and testing without physical hardware.

## Overview

The template includes QEMU emulation support that allows you to:

- Test your application without physical ESP32 hardware
- Debug with GDB in a virtual environment
- Simulate network communication via UART bridge
- Develop and iterate faster without hardware constraints

When building for the emulator, you can optionally create simulator implementations of your hardware components that maintain identical APIs to the real hardware versions. This allows the same application code to run in both hardware and emulated environments without modification.

## Architecture

### Hardware Abstraction Strategy (Optional)

When you need to simulate hardware components, you can use a **clean separation approach**:

- **Same headers**: Identical APIs for both hardware and simulator versions
- **Different source files**: CMake selects appropriate implementation at build time
- **No #ifdef clutter**: Clean, maintainable code without conditional compilation

This is entirely optional - the template's minimal main.c doesn't require any hardware simulation.

### Network Stack (Included)

- **Full Network Implementation**: Complete TCP/IP stack via UART-based IP tunnel
- **TUN Device Bridge** (`tools/serial_tun_bridge.py`): Bridges QEMU UART1 to Linux TUN interface
  - ESP32 address: `192.168.100.2/24`
  - Host TUN interface: `192.168.100.1/24`
  - Ethernet frame encapsulation for lwIP compatibility
- **HTTP Proxy** (`tools/http_proxy.py`): Forwards `localhost:8080` → ESP32 web server
  - Enables browser access via GitHub Codespaces port forwarding
  - Automatic retry with exponential backoff
  - Handles ESP32 restarts gracefully
- **Web Interface**: Fully accessible via network tunnel (real HTTP connections, not localhost mock)
- **See Also**: [Network Internals Documentation](development/qemu-network-internals.md) for deep dive

## Build Configuration

### Kconfig Options

Configuration options in `main/Kconfig.projbuild`:

```kconfig
config TARGET_EMULATOR
    bool "Build for QEMU emulator"
    default n
    help
        Enable this option to build for QEMU emulator instead of real hardware

config EMULATOR_MOCK_DATA
    bool "Use mocked data in emulator"
    depends on TARGET_EMULATOR
    default y
    help
        Generate simulated data for testing
```

### CMake Integration

The build system automatically selects the correct source files:

**Distance Sensor** (`components/distance_sensor/CMakeLists.txt`):

```cmake
if(CONFIG_TARGET_EMULATOR)
    set(COMPONENT_SRCS "distance_sensor_sim.c")
else()
    set(COMPONENT_SRCS "distance_sensor.c")
endif()
```

**LED Controller** (`components/led_controller/CMakeLists.txt`):

```cmake
if(CONFIG_TARGET_EMULATOR)
    set(COMPONENT_SRCS "led_controller_sim.c")
else()
    set(COMPONENT_SRCS "led_controller.c")
endif()
```

## Usage Instructions

### 1. Hardware Build (Default)

```bash
idf.py build
idf.py flash monitor
```

### 2. Emulator Build

```bash
# Configure for emulator
idf.py menuconfig
# Navigate to: ESP32 Distance Project Configuration
# Enable: [x] Build for QEMU emulator
# Enable: [x] Use mocked sensor data in emulator

# Build
idf.py build

# Run in QEMU (example)
qemu-system-xtensa -nographic -M esp32 -kernel build/distance.elf
```

## Expected Output

### Console Logs

```text
I (1000) main: ESP32 Distance Measurement with LED Strip Display
I (1100) led_controller_sim: LED controller simulator initialized: 40 LEDs (terminal visualization)
I (1200) distance_sensor_sim: Distance sensor simulator initialized successfully  
I (1300) distance_sensor_sim: Distance sensor simulator started (5cm→60cm→5cm sweep, interval: 1000 ms)
```

### Animated Distance Readings

```text
I (2000) distance_sensor_sim: Simulated distance: 5.0 cm (increasing)
I (3000) distance_sensor_sim: Simulated distance: 5.1 cm (increasing)
I (4000) distance_sensor_sim: Simulated distance: 5.2 cm (increasing)
...
I (58000) distance_sensor_sim: Simulated distance: 60.0 cm (decreasing)
I (59000) distance_sensor_sim: Simulated distance: 59.9 cm (decreasing)
```

### LED Strip Visualization

```text
[LED Strip]: 🔴🔴🔴⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫

[LED Strip]: 🟡🟡🟡🟡🟡🟡🟡🟡⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫

[LED Strip]: 🟢🟢🟢🟢🟢🟢🟢🟢🟢🟢🟢🟢⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫

[LED Strip]: 🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫
```

### Color Mapping Legend

- 🔴 **Red**: Close distances (5-15cm)
- 🟡 **Yellow**: Medium distances (15-35cm)
- 🟢 **Green**: Far distances (35-50cm)
- 🔵 **Blue**: Maximum distances (50-60cm)
- ⚪ **White**: Very bright/mixed colors
- 🟣 **Purple**: Magenta/mixed colors
- ⚫ **Black**: Off/very dim LEDs

## Implementation Details

### Distance Sensor Simulator

**Animation Logic**:

```c
// Animate distance: 5cm (50mm) → 60cm (600mm) → 5cm (50mm)
sim_distance += direction;

if (sim_distance >= 600) {  // 60.0cm = 600mm
    direction = -1;
} else if (sim_distance <= 50) {  // 5.0cm = 50mm  
    direction = 1;
}
```

**Queue Behavior**: Identical to hardware implementation with overflow handling and statistics tracking.

### LED Controller Simulator  

**Rate Limiting**:

```c
static uint64_t last_display_time = 0;
static const uint64_t DISPLAY_INTERVAL_US = 1000000; // 1 second

esp_err_t led_show(void) {
    uint64_t now = esp_timer_get_time();
    if (now - last_display_time < DISPLAY_INTERVAL_US) {
        return ESP_OK;  // Suppress output, just return success
    }
    last_display_time = now;
    // ... display emoji output
}
```

**Color Analysis**: Intelligent mapping from RGB values to representative emoji blocks based on dominant colors and brightness levels.

## Benefits

### Development Advantages

- **No Hardware Dependencies**: Develop without physical ESP32, sensors, or LEDs
- **Fast Iteration**: Quick build-test cycles without flashing hardware
- **Visual Feedback**: Clear terminal visualization of system behavior
- **CI/CD Friendly**: Automated testing in continuous integration pipelines
- **Cross-Platform**: Develop on any system with QEMU support

### Testing Advantages

- **Predictable Behavior**: Animated patterns for systematic testing
- **Integration Testing**: Full system testing without hardware setup
- **Algorithm Validation**: Test LED display logic and distance mapping
- **Performance Analysis**: Monitor queue behavior and timing

### Educational Benefits

- **System Understanding**: Clear visualization of sensor-to-display pipeline
- **Algorithm Learning**: See distance mapping and color gradients in action
- **Real-time Concepts**: Observe queue-based architecture behavior

## Files Modified/Created

### New Files

- `main/Kconfig.projbuild` - Build configuration options
- `main/components/distance_sensor/distance_sensor_sim.c` - Distance sensor simulator
- `main/components/led_controller/led_controller_sim.c` - LED controller simulator

### Modified Files

- `main/components/distance_sensor/CMakeLists.txt` - Conditional source selection
- `main/components/led_controller/CMakeLists.txt` - Conditional source selection

### Unchanged Files

- All header files (`.h`) - APIs remain identical
- Main application logic - No changes needed
- WiFi/web server components - Run unchanged

## Validation

The implementation has been validated for:

- ✅ Complete API compatibility with hardware versions
- ✅ Proper CMake conditional compilation
- ✅ Animation logic and timing behavior  
- ✅ LED visualization and rate limiting
- ✅ Queue-based architecture and error handling
- ✅ No security vulnerabilities introduced

## Future Enhancements

Potential improvements for the emulator support:

- **Web Interface Preview**: Show LED state in browser
- **Interactive Controls**: Manual distance adjustment via keyboard
- **Performance Metrics**: Queue utilization and timing statistics
- **Multiple Animation Patterns**: Different test sequences
- **Color Scheme Options**: Alternative emoji sets or ASCII mode

---

*This emulator support enables efficient development and testing of the ESP32 distance sensor project without requiring physical hardware, while maintaining complete compatibility with the production system.*
