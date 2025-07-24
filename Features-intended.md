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
   - Implement proper component encapsulation
   - Simplify main.c to initialization-only

2. **Second Priority**: HTTP Web Server & Data Integration
   - Complete HTTP web server implementation
   - Connect web server to live sensor and LED data
   - Implement planned API endpoints and web interface features

3. **After Core Features**: Pick next features from `features-intended.md`
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

### Configuration Management System 💭 **HIGH PRIORITY**

> **🎯 AI Development Demonstration**: This requirement is designated for implementation using **GitHub Copilot** as part of our AI-assisted embedded development showcase.

**Objective**: Consolidate all magic numbers into a centralized configuration system with runtime modification capabilities and persistent storage.

**📋 Formal Requirements**: See [`docs/requirements/config-requirements.md`](docs/requirements/config-requirements.md) for detailed OpenFastTrack requirements specification.

**🏗️ Design Specification**: See [`docs/design/config-design.md`](docs/design/config-design.md) for architecture and implementation design.

**🧪 Test Specification**: See [`docs/test/config-tests.md`](docs/test/config-tests.md) for comprehensive test cases and validation.

**Implementation Phases**:

1. **Phase 1** (GitHub Copilot): Magic number consolidation and source code migration
2. **Phase 2** (GitHub Copilot): NVS storage implementation and configuration API  
3. **Phase 3** (Human-AI Collaboration): Web interface development and advanced features

**Key Requirements Summary**:

- **REQ-CFG-1**: Centralize all hardcoded values into `main/config.h`
- **REQ-CFG-3**: Runtime configuration structure with validation ranges
- **REQ-CFG-4**: NVS persistent storage with corruption detection
- **REQ-CFG-5**: Thread-safe configuration API with error handling
- **REQ-CFG-7**: Web interface for runtime parameter modification

**Benefits for AI Development Demo**:

- **Structured Requirements**: Enable precise GitHub Copilot implementation guidance
- **Traceability**: Demonstrate requirement→design→code→test links with AI assistance
- **Quality Assurance**: Show formal validation of AI-generated embedded code
- **Safety Integration**: Prove AI tools work within safety-critical development frameworks

**Identified Magic Numbers to Consolidate:**

```c
// main/config.h - Centralized Configuration
#ifndef CONFIG_H
#define CONFIG_H

// === DISTANCE SENSOR CONFIGURATION ===
#define DEFAULT_DISTANCE_MIN_CM         10.0f      // Minimum distance for LED mapping
#define DEFAULT_DISTANCE_MAX_CM         50.0f      // Maximum distance for LED mapping  
#define DEFAULT_DISTANCE_INTERVAL_MS    100        // Measurement interval (10Hz)
#define DEFAULT_DISTANCE_TIMEOUT_MS     30         // Echo timeout
#define DEFAULT_TEMPERATURE_C           20.0f      // Room temperature for sound speed
#define DEFAULT_SMOOTHING_ALPHA         0.3f       // EMA smoothing factor

// === LED CONTROLLER CONFIGURATION ===
#define DEFAULT_LED_COUNT               40         // Number of LEDs in strip
#define DEFAULT_LED_RMT_CHANNEL         0          // RMT channel for LED control
#define DEFAULT_LED_BRIGHTNESS          128        // LED brightness (0-255)
#define DEFAULT_LED_TEST_DELAY_MS       50         // LED test animation delay

// WS2812 Timing (hardware-specific, rarely changed)
#define WS2812_T0H_TICKS               32         // 0.4us high for bit 0
#define WS2812_T0L_TICKS               64         // 0.8us low for bit 0  
#define WS2812_T1H_TICKS               64         // 0.8us high for bit 1
#define WS2812_T1L_TICKS               32         // 0.4us low for bit 1
#define WS2812_RESET_TICKS             4000       // 50us reset pulse

// === WIFI CONFIGURATION ===
#define DEFAULT_WIFI_AP_CHANNEL         1          // AP mode channel
#define DEFAULT_WIFI_AP_MAX_CONN        4          // Max AP connections
#define DEFAULT_WIFI_STA_MAX_RETRY      3          // STA connection retries
#define DEFAULT_WIFI_STA_TIMEOUT_MS     5000       // STA connection timeout
#define DEFAULT_WIFI_SCAN_MIN_MS        100        // WiFi scan time minimum
#define DEFAULT_WIFI_SCAN_MAX_MS        300        // WiFi scan time maximum

// === WEB SERVER CONFIGURATION ===
#define DEFAULT_HTTP_PORT               80         // HTTP server port
#define DEFAULT_HTTPS_PORT              443        // HTTPS server port (future)
#define DEFAULT_MAX_URI_LENGTH          64         // Maximum URI buffer size
#define DEFAULT_IP_STRING_LENGTH        16         // IP address string buffer
#define DEFAULT_TASK_DELAY_MS           1000       // General task delay

// === SYSTEM TIMING CONFIGURATION ===
#define DEFAULT_MONITOR_INTERVAL_MS     5000       // System monitor loop interval
#define DEFAULT_MODE_SWITCH_DELAY_MS    100        // WiFi mode switch delay
#define DEFAULT_QUEUE_RAW_SIZE          2          // ISR queue size (minimal)
#define DEFAULT_QUEUE_PROCESSED_SIZE    5          // API queue size (buffering)

#endif // CONFIG_H
```

**Phase 2: Runtime Configuration System**
- **NVS Storage**: Implement persistent configuration storage in ESP32 flash
- **Config Structure**: Define runtime configuration with validation ranges
- **Load/Save API**: Functions to load defaults, save changes, and validate ranges
- **Factory Reset**: Restore all values to compile-time defaults

```c
// Runtime configuration structure (stored in NVS)
typedef struct {
    // Distance sensor settings
    float distance_min_cm;           // Range: 5.0 - 100.0
    float distance_max_cm;           // Range: 20.0 - 400.0
    uint16_t measurement_interval_ms; // Range: 50 - 1000
    uint32_t sensor_timeout_ms;      // Range: 10 - 100
    float smoothing_alpha;           // Range: 0.1 - 1.0
    
    // LED settings  
    uint8_t led_count;               // Range: 1 - 60
    uint8_t led_brightness;          // Range: 10 - 255
    
    // WiFi settings
    uint8_t wifi_max_retry;          // Range: 1 - 10
    uint32_t wifi_timeout_ms;        // Range: 1000 - 30000
    
    // Web server settings
    uint32_t monitor_interval_ms;    // Range: 1000 - 60000
    
    // System info
    uint32_t config_version;         // For migration compatibility
    uint32_t save_count;             // Track configuration changes
} system_config_t;

// Configuration API
esp_err_t config_init(void);
esp_err_t config_load(system_config_t* config);
esp_err_t config_save(const system_config_t* config);
esp_err_t config_factory_reset(void);
bool config_validate_range(const system_config_t* config);
```

**Phase 3: Web Configuration Interface**
- **Settings Page**: Web interface for runtime parameter modification
- **Real-time Preview**: Live updates showing effect of changes before saving
- **Validation**: Client and server-side range validation with user feedback
- **Backup/Restore**: Export/import configuration as JSON

**Web Interface Features:**
- **Distance Calibration**: Set measurement range and LED mapping
- **LED Settings**: Brightness, count, test animations
- **Sensor Tuning**: Timeout, smoothing, measurement frequency
- **WiFi Management**: Retry counts, timeout values
- **System Settings**: Monitor intervals, factory reset option

**Implementation Priority:**
1. **Phase 1** (Immediate): Consolidate magic numbers into config.h
2. **Phase 2** (Next): Implement NVS storage and runtime configuration
3. **Phase 3** (Future): Web interface for configuration management

**Benefits:**

- **Maintainability**: All configuration in one location
- **Flexibility**: Runtime tuning without firmware recompilation  
- **User Experience**: Web-based configuration for end users
- **Debugging**: Easy parameter adjustment during development
- **Production**: Field-configurable devices without code changes

---

### Shared Data Architecture for Sensor→Web Integration 💭 **HIGH PRIORITY**

**Objective**: Implement thread-safe data sharing between sensor monitoring task and web server for real-time data access.

**Current Challenge**: Web server needs access to live sensor data, but sensor monitoring runs in separate FreeRTOS task.

**Solution**: Mutex-protected shared data structure with atomic updates and non-blocking reads.

```c
// main/shared_data.h - Thread-safe data sharing
#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_timer.h"

// Sensor status enumeration
typedef enum {
    DISTANCE_STATUS_OK = 0,
    DISTANCE_STATUS_TIMEOUT,
    DISTANCE_STATUS_OUT_OF_RANGE,
    DISTANCE_STATUS_ERROR
} distance_status_t;

// LED strip state for web visualization
typedef struct {
    uint8_t red;
    uint8_t green; 
    uint8_t blue;
    bool active;
} led_state_t;

// Shared data structure (protected by mutex)
typedef struct {
    // Distance sensor data
    float distance_cm;                    // Current distance reading
    float smoothed_distance_cm;           // EMA smoothed distance
    distance_status_t status;             // Sensor status
    uint64_t timestamp_us;                // Timestamp in microseconds
    uint32_t measurement_count;           // Total measurements taken
    uint32_t error_count;                 // Number of failed readings
    
    // LED strip state (for web visualization)
    led_state_t led_states[60];           // LED strip state array
    uint8_t active_led_count;             // Number of active LEDs
    
    // System health
    uint32_t uptime_seconds;              // System uptime
    uint32_t heap_free_bytes;             // Available heap memory
    int8_t wifi_rssi;                     // WiFi signal strength
} shared_data_t;

// Shared data API
esp_err_t shared_data_init(void);
esp_err_t shared_data_update_distance(float distance_cm, distance_status_t status);
esp_err_t shared_data_update_leds(const led_state_t* led_states, uint8_t count);
esp_err_t shared_data_update_system(uint32_t uptime, uint32_t heap_free, int8_t rssi);
esp_err_t shared_data_get_snapshot(shared_data_t* snapshot);
void shared_data_cleanup(void);

#endif // SHARED_DATA_H
```

**Implementation Details:**

```c
// main/shared_data.c - Implementation
static shared_data_t g_shared_data = {0};
static SemaphoreHandle_t g_data_mutex = NULL;

esp_err_t shared_data_init(void) {
    g_data_mutex = xSemaphoreCreateMutex();
    if (g_data_mutex == NULL) {
        return ESP_ERR_NO_MEM;
    }
    
    // Initialize with default values
    if (xSemaphoreTake(g_data_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_shared_data.status = DISTANCE_STATUS_OK;
        g_shared_data.timestamp_us = esp_timer_get_time();
        xSemaphoreGive(g_data_mutex);
    }
    
    return ESP_OK;
}

// Non-blocking read with timeout
esp_err_t shared_data_get_snapshot(shared_data_t* snapshot) {
    if (snapshot == NULL) return ESP_ERR_INVALID_ARG;
    
    if (xSemaphoreTake(g_data_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(snapshot, &g_shared_data, sizeof(shared_data_t));
        xSemaphoreGive(g_data_mutex);
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT; // Web server can handle stale data
}
```

**Integration Points:**

1. **Sensor Task Updates**: 
   ```c
   // In distance monitoring task
   shared_data_update_distance(distance_cm, status);
   ```

2. **LED Controller Updates**:
   ```c
   // After LED strip update
   shared_data_update_leds(current_led_states, led_count);
   ```

3. **Web Server Reads**:
   ```c
   // In HTTP request handler
   shared_data_t current_data;
   if (shared_data_get_snapshot(&current_data) == ESP_OK) {
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
