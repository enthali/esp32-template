# ESP32 Distance Project - Feature Intentions

This document contains **unnumbered** feature intentions that may or may not be implemented. These are flexible ideas that can be reordered, modified, or moved to the planned roadmap as priorities become clear.
---

## HTTPS Security Implementation 📋 **NEXT AFTER COMPONENT RESTRUCTURING**  
- 📋 **HTTPS Server**: Replace HTTP with encrypted HTTPS using ESP32 SSL/TLS support
- 📋 **Self-Signed Certificates**: Generate and embed certificates for local IoT device use
- 📋 **Certificate Generation**: Build-time certificate creation and embedding
- 📋 **Mixed Mode Support**: HTTPS for production, HTTP fallback for development
- 📋 **Browser Compatibility**: Handle self-signed certificate warnings appropriately

**Security Benefits:**
- **Encrypted Transmission**: All WiFi credentials and sensor data protected in transit
- **Man-in-the-Middle Protection**: Prevents network eavesdropping attacks  
- **Cross-Site Attack Prevention**: Combined with CORS removal for comprehensive security
- **IoT Security Best Practice**: Industry standard for connected devices

**Implementation Plan:**
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

**Certificate Strategy:**
- **Self-Signed Certificates**: Perfect for local IoT devices (no external CA needed)
- **Build-Time Generation**: Automated certificate creation during ESP-IDF build
- **10-Year Validity**: Long-lived certificates for device lifecycle
- **Browser Warnings**: Users manually accept certificate (standard IoT practice)

**Deliverables:**
- HTTPS server implementation replacing HTTP
- Automated certificate generation and embedding
- Updated web interface to use https:// URLs
- Documentation for certificate acceptance procedure
- Secure transmission of all WiFi credentials and sensor data

---

## Development Priorities

1. **First Priority**: Component Architecture Restructuring
   - Clean up the current codebase structure
   - Implement proper component encapsulation
   - Simplify main.c to initialization-only

2. **Second Priority**: HTTPS Security Implementation
   - Add encryption to the web interface
   - Secure WiFi credential transmission
   - Complete the core security foundation

3. **After HTTPS**: Pick next features from `features-intended.md`
   - Configuration management
   - Real-time data streaming
   - JSON API endpoints
   - Advanced features as needed

## Architecture Goals

The current roadmap focuses on:
- **Clean Architecture**: Proper component separation and encapsulation
- **Security**: HTTPS encryption for all web communications
- **Maintainability**: Simplified main.c and self-contained components
- **Foundation**: Solid base for future feature additions

Once these foundational improvements are complete, we'll have a clean, secure, and maintainable codebase ready for the next phase of features from the intended roadmap.


## Web Interface Intentions 💭

### Configuration Management & Data Sharing 💭 **INTENDED**
- **Configuration System**: Centralize magic numbers into `main/config.h` with NVS storage
- **Shared Data Structure**: Implement mutex-protected shared variable for sensor→web data flow
- **Runtime Configuration**: Store user-configurable parameters in NVS flash
- **Default Values**: Compile-time defaults with runtime override capability

**Configuration Categories:**
```c
// config.h - Compile-time defaults
#define DEFAULT_DISTANCE_MIN_CM         10
#define DEFAULT_DISTANCE_MAX_CM         50  
#define DEFAULT_LED_COUNT               40
#define DEFAULT_MEASUREMENT_INTERVAL_MS 100
#define DEFAULT_LED_BRIGHTNESS          128

// Runtime config structure (stored in NVS)
typedef struct {
    uint16_t distance_min_cm;
    uint16_t distance_max_cm;
    uint8_t led_brightness;
    uint16_t measurement_interval_ms;
    uint32_t sensor_timeout_ms;
} system_config_t;
```

**Shared Data Architecture:**
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

## Production & Deployment Intentions 💭

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
```
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
4. **Fun Features**: Garage assistant (when core is complete)

## Architecture Considerations 💭

All intended features should maintain clean architecture:
- **Component Encapsulation**: Self-contained components
- **Security First**: Use HTTPS foundation
- **Real-time Friendly**: Work with FreeRTOS task architecture
- **Configuration Driven**: Configurable via web interface
- **Memory Conscious**: ESP32 memory constraints

## Migration from Intentions to Plans 💭

When an intention becomes a plan:
1. Move the feature to `Features-planned.md` with assigned step number
2. Add release timeline and specific deliverables
3. Create detailed implementation plan
4. Remove from this intentions document

This ensures clear separation between "what we might do" vs. "what we will do".