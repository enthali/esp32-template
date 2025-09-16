# ESP32 Distance Project - Feature Intentions

This document contains **unnumbered** feature intentions that may or may not be implemented. These are flexible ideas that can be reordered, modified, or moved to the planned roadmap as priorities become clear.

---

## HTTPS Security Implementation 💭 **PARKED FOR FUTURE**

- 💭 **HTTPS Server**: Replace HTTP with encrypted HTTPS using ESP32 SSL/TLS support
- 💭 **Self-Signed Certificates**: Generate and embed certificates for local IoT device use
- 💭 **Certificate Generation**: Build-time certificate creation and embedding
- 💭 **Mixed Mode Support**: HTTPS for production, HTTP fallback for development
- 💭 **Browser Compatibility**: Handle self-signed certificate warnings appropriately

**Status:**

- HTTPS implementation is currently on hold due to complexity and time constraints.
- Certificate generation scripts and groundwork are retained for future use.
- Focus is now on HTTP web server, live data integration, and core features.

**Security Benefits (when implemented):**

- Encrypted transmission of WiFi credentials and sensor data
- Man-in-the-middle protection
- Cross-site attack prevention
- IoT security best practice

**Implementation Plan (for future reference):**

```c
#include "esp_https_server.h"
#include "esp_tls.h"

// Embedded certificate files (generated at build time)
extern const uint8_t servercert_start[] asm("_binary_servercert_pem_start");
extern const uint8_t servercert_end[]   asm("_binary_servercert_pem_end");
extern const uint8_t prvtkey_start[]    asm("_binary_prvtkey_pem_start");
extern const uint8_t prvtkey_end[]      asm("_binary_prvtkey_pem_end");

// HTTPS server configuration
httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();
conf.servercert = servercert_start;
conf.servercert_len = servercert_end - servercert_start;
conf.prvtkey_pem = prvtkey_start;
conf.prvtkey_len = prvtkey_end - prvtkey_start;
conf.httpd.server_port = 443;
```

---

## Development Priorities

1. **First Priority**: Component Architecture Restructuring
   - Clean up the current codebase structure
## Summary of Implemented Intentions

All features described in this document up to `Production & Deployment Intentions` have been implemented or scaffolded. The implemented areas include:

- Component architecture refactor and modularization (sensor, LED, WiFi, web server components)
- Distance sensor integration and robust monitoring with FreeRTOS tasks
- LED visualization and display logic with WS2812 control and mapping
- Web interface foundation: captive portal, static pages embedded in flash, and settings endpoints
- Configuration consolidation plan and NVS-backed persistence APIs scaffolded
- Shared data architecture for safe sensor→web data sharing
- Basic roadmap items (JSON API, SSE, OTA scaffolding) are documented and available as intended features

HTTPS support and production/deployment topics remain as intentions and are preserved below for future work.
       // Use current_data for response
   }
   ```

4. **System Monitor Updates**:

   ```c
   // In system monitor task
   shared_data_update_system(uptime, esp_get_free_heap_size(), wifi_rssi);
   ```

**Thread Safety Guarantees:**

- **Mutex Protection**: All access protected by FreeRTOS mutex
- **Short Critical Sections**: Minimal time holding mutex to prevent blocking
- **Non-blocking Reads**: Web server gets timeout instead of indefinite wait  
- **Atomic Updates**: Complete data structure copied to prevent partial reads
- **Fallback Handling**: Web server can use previous data if mutex unavailable

**Performance Considerations:**

- **10ms Timeout**: Web server timeout prevents blocking user requests
- **Memory Copy**: Full structure copy ensures consistency but uses stack space
- **Update Frequency**: Sensor updates at 10Hz, web requests typically <1Hz
- **Priority Inversion**: Mutex priority inheritance prevents low-priority blocking

```c
// Sensor → Web Server data sharing
typedef struct {
    float distance_cm;
    uint32_t timestamp;
    distance_status_t status;
} distance_data_t;

static distance_data_t shared_distance_data;
static SemaphoreHandle_t distance_mutex;
```

**Integration Points:**

- Replace all magic numbers with config values
- Sensor task updates shared data structure (protected by mutex)
- Web server reads shared data structure (protected by mutex)
- Configuration changes trigger system parameter updates

---

### Real-time Data Streaming 💭 **INTENDED**

- **Server-Sent Events (SSE)**: Real-time distance updates
- **Live Dashboard**: Auto-updating web interface without page refresh
- **LED Visualization**: Browser-based LED strip representation
- **WebSocket Support**: Bidirectional communication (optional upgrade)

**Real-time Features:**

```c
// SSE endpoint for live updates
GET /events -> text/event-stream
data: {"distance": 25.4, "led_states": [...], "timestamp": 1234567890}
```

**Dashboard Features:**

- Live distance number with units
- Virtual LED strip matching physical hardware
- Color-coded status indicators
- Real-time error notifications

---

### JSON API Endpoints 💭 **INTENDED**

- **RESTful API**: JSON endpoints for programmatic access
- **Core Endpoints** (flat structure):
  - `GET /api/distance` - Current distance measurement
  - `GET /api/status` - System health and statistics  
  - `GET /api/config` - Current configuration
  - `POST /api/config` - Update settings
- **Error Handling**: Proper HTTP status codes and error messages

**API Design:**

```c
// Example JSON responses
GET /api/distance -> {"distance": 25.4, "status": "ok", "timestamp": 1234567890}
GET /api/status   -> {"uptime": 3600, "wifi": "connected", "tasks": "healthy"}
```

---

### Advanced Configuration Interface 💭 **INTENDED**

- **Settings Management**: Web-based configuration for all system parameters
- **Distance Calibration**: Set min/max ranges for LED mapping  
- **LED Configuration**: Brightness, color schemes, animation modes
- **Sensor Settings**: Timeout values, temperature compensation
- **WiFi Management**: Change networks, view connection status
- **System Controls**: Restart, factory reset, firmware updates

**Configuration Categories:**

- **Distance Mapping**: 10-50cm range, LED assignment, error thresholds
- **Visual Settings**: LED brightness, color patterns, error indicators  
- **Sensor Tuning**: Measurement intervals, timeout handling, temperature
- **Network Settings**: WiFi credentials, static IP, hostname
- **System Maintenance**: Logs, diagnostics, updates

---

### Production & Deployment Intentions 💭

### OTA Firmware Updates 💭 **INTENDED**

- **Over-The-Air Updates**: ESP32 OTA partition scheme and update mechanism
- **Version Management**: Firmware versioning and rollback capability
- **Update Server**: Simple HTTP/HTTPS server for firmware distribution
- **Security**: Signed firmware updates and secure boot
- **User Interface**: Web-based firmware update with progress indication

### Security Hardening 💭 **INTENDED**

- **WiFi Security**: WPA3 support and strong encryption
- **Web Interface Security**: HTTPS, session management, CSRF protection
- **Access Control**: Basic authentication for configuration pages
- **Network Security**: Firewall rules and secure communication
- **Credential Protection**: Encrypted storage of sensitive data

### Performance Optimization 💭 **INTENDED**

- **Memory Management**: Heap usage optimization and leak detection
- **Task Optimization**: CPU usage profiling and optimization
- **Network Performance**: HTTP server optimization and caching
- **Power Management**: Sleep modes and power consumption optimization
- **Storage Optimization**: Flash usage and wear leveling

### MQTT Integration 💭 **INTENDED**

- **MQTT Client**: Connect to MQTT broker for IoT integration
- **Data Publishing**: Real-time distance measurements to MQTT topics
- **Configuration**: MQTT broker settings via web interface
- **Status Reporting**: System health and diagnostics via MQTT
- **Home Automation**: Integration with Home Assistant, OpenHAB, etc.

**MQTT Topics:**

```text
esp32-distance/distance     - Current distance measurement
esp32-distance/status       - System status and health
esp32-distance/config       - Configuration updates
esp32-distance/led/state    - LED strip status
```

### System Reliability 💭 **INTENDED**

- **Watchdog Timers**: Hardware and software watchdog implementation
- **Error Recovery**: Comprehensive error handling and recovery mechanisms
- **Logging System**: Persistent log storage and remote log access
- **Health Monitoring**: System metrics and performance monitoring
- **Factory Reset**: Complete system reset capability

---

## Garage Parking Assistant (Optional) 💭

### Multi-Zone Detection 💭 **INTENDED**

- **Multiple Sensors**: Support for multiple HC-SR04 sensors
- **Zone Configuration**: Configurable detection zones (approach, stop, too close)
- **Sensor Fusion**: Combine multiple sensor readings for accuracy
- **Parking Guidance**: Visual indicators for optimal parking position
- **Vehicle Detection**: Distinguish between vehicles and other objects

### Audio Alert System 💭 **INTENDED**

- **Piezo Buzzer**: Audio feedback for parking guidance (if piezo found! 🔍)
- **Alert Patterns**: Different beep patterns for different zones
- **Volume Control**: Configurable audio levels via web interface
- **Silent Mode**: Option to disable audio alerts
- **Custom Sounds**: Configurable alert tones and patterns

### Advanced Parking Logic 💭 **INTENDED**

- **Car Detection**: Algorithms to detect vehicle presence vs. absence
- **Parking Position**: Optimal stop position calculation
- **Multiple Vehicles**: Support for different vehicle sizes
- **Learning Mode**: Adaptive algorithms that learn parking preferences
- **Safety Zones**: Configurable safety margins and warning zones

### Garage Integration 💭 **INTENDED**

- **Door Sensors**: Integration with garage door position sensors
- **Automation**: Automatic garage door control (future expansion)
- **Multiple Bays**: Support for multi-bay garages
- **User Profiles**: Different settings for different drivers
- **Mobile Notifications**: Push notifications for parking events

**Hardware Requirements:**

- Multiple HC-SR04 sensors (2-4 units)
- Piezo buzzer (if available from "many years back" 😄)
- Optional: Garage door position sensors
- Optional: Additional LED strips for multi-zone indication

**Use Cases:**

- Single-car garage parking assistant
- Multi-bay garage with individual guidance
- RV/boat parking assistance
- Workshop/storage area object positioning

---

## Priority Guidelines 💭

These intentions have **no fixed priority**. When ready to implement:

1. **High Interest**: Configuration management, real-time streaming
2. **Medium Interest**: JSON API, advanced configuration
3. **Variable Interest**: Production features (depends on deployment needs)
4. **Fun Features**: Garage assistant (when core is complete
