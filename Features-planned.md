# ESP32 Distance Project - Current Development Roadmap

This document contains the immediate next steps for the ESP32 Distance Project. Once these are completed, we'll pick the next items from `FUTURE.md`.

## Current Development Status

**All core functionality is COMPLETED** ✅ - See `COMPLETED.md` for details:
- LED Strip Animation ✅
- Ultrasonic Sensor Integration ✅  
- Distance-to-LED Mapping ✅
- WiFi Setup with Smart Network Logic ✅
- Basic Static Web Interface ✅

## Next Development Steps

### Step 4.2.5: Component Architecture Restructuring 📋 **NEXT**  
- 📋 **Component Encapsulation**: Move monitoring logic into respective components for complete self-containment
- 📋 **WiFi Component**: Restructure `wifi_manager` into `wifi/` folder with internal monitoring
- 📋 **Distance Sensor Enhancement**: Add internal sensor health monitoring to `distance_sensor/` component
- 📋 **Simplified Main**: Remove all monitoring loops from `main.c` - components handle their own health internally
- 📋 **Internal Monitoring Tasks**: Each component starts its own low-priority monitoring task automatically

**Architecture Benefits:**
- **Complete Encapsulation**: Each component fully responsible for its own health and monitoring
- **Simplified Main**: `main.c` becomes initialization-only with no monitoring loops
- **Internal Implementation Detail**: Monitoring hidden from component consumers
- **Self-Contained Components**: Each component manages its lifecycle independently
- **Clean APIs**: Public APIs stay focused on core functionality

**Proposed Structure:**
```
📁 components/
├── 📁 wifi/                   # All WiFi-related functionality
│   ├── wifi_manager.h         # Management API (existing)
│   ├── wifi_manager.c         # WiFi management (existing)
│   ├── wifi_monitor.h         # Internal monitoring (new)
│   └── wifi_monitor.c         # Status monitoring task (new)
├── 📁 distance_sensor/        # All sensor functionality  
│   ├── distance_sensor.h      # Sensor API (existing)
│   ├── distance_sensor.c      # Sensor driver (existing)
│   ├── sensor_monitor.h       # Internal monitoring (new)
│   └── sensor_monitor.c       # Health monitoring task (new)
└── 📁 display/                # All display functionality
    ├── display_logic.h        # Display API (existing)
    └── display_logic.c        # Display logic (existing)
    # Note: Display monitoring optional - no current monitoring needs
```

**Implementation Strategy:**
- `wifi_manager_init()` internally starts wifi_monitor for status logging
- `distance_sensor_init()` internally starts sensor_monitor for queue overflow tracking
- Monitoring tasks run at low priority (1-2) and are transparent to consumers
- Public APIs unchanged - monitoring is internal implementation detail
- Main.c becomes pure initialization with no runtime monitoring loops

**Clean Main.c Result:**
```c
void app_main(void) {
    // Hardware initialization only - monitoring starts automatically
    led_controller_init(&led_config);
    distance_sensor_init(&distance_config);  // Starts internal monitoring
    wifi_manager_init();                      // Starts internal monitoring  
    display_logic_init(&display_config);
    
    // Start services
    distance_sensor_start();
    wifi_manager_start();
    display_logic_start();
    
    ESP_LOGI(TAG, "All systems initialized and running");
    
    // Main just sleeps - no monitoring loops needed!
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

**Deliverables:**
- Restructured `wifi/` component with internal monitoring
- Enhanced `distance_sensor/` component with internal health monitoring  
- Cleaned `main.c` with initialization-only logic
- Component-internal monitoring tasks (transparent to consumers)
- Updated component APIs with automatic monitoring lifecycle

---

### Step 4.2.6: HTTPS Security Implementation 📋 **NEXT AFTER RESTRUCTURING**  
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

3. **After HTTPS**: Pick next features from `FUTURE.md`
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

Once these foundational improvements are complete, we'll have a clean, secure, and maintainable codebase ready for the next phase of features from the future roadmap.