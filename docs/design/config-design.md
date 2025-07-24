# Configuration System Design Specification

**Document ID**: DSN-CFG-ESP32-DISTANCE  
**Version**: 1.0  
**Date**: 2025-07-24  
**Author**: ESP32 Distance Project Team  
**Requirements Traceability**: REQ-CFG-1 through REQ-CFG-11  

## Design Overview

This document specifies the design for the Configuration Management System, covering architecture, data structures, API design, and implementation approach.

## Architecture Design

### DSN-CFG-1: Layered Configuration Architecture
**Covers**: REQ-CFG-1, REQ-CFG-2  
**Type**: System Architecture  

The configuration system implements a three-layer architecture:

```
┌─────────────────────────────────────────┐
│           Application Layer             │
│  (main.c, components, web_server.c)     │
├─────────────────────────────────────────┤
│         Configuration API Layer         │
│    (config.h, config.c - Public API)    │
├─────────────────────────────────────────┤
│           Storage Layer                 │
│     (NVS abstraction, validation)       │
└─────────────────────────────────────────┘
```

**Layer Responsibilities**:

- **Application Layer**: Consumes configuration values through standardized API
- **Configuration API Layer**: Provides thread-safe access, validation, and caching
- **Storage Layer**: Handles NVS operations, persistence, and error recovery

### DSN-CFG-2: Configuration Data Flow
**Covers**: REQ-CFG-3, REQ-CFG-4, REQ-CFG-5  
**Type**: Data Flow Design  

```
Startup:     NVS → config_load() → Runtime Cache → Application
Runtime:     Application → config_get() → Runtime Cache
Update:      Web Interface → config_save() → Validation → NVS → Runtime Cache
Factory:     config_factory_reset() → Defaults → NVS → Runtime Cache
```

**Key Design Decisions**:

1. **Runtime Cache**: Configuration cached in RAM for fast access
2. **Atomic Updates**: Configuration changes applied atomically
3. **Validation Gate**: All changes validated before persistence
4. **Error Recovery**: Automatic fallback to defaults on corruption

## Data Structure Design

### DSN-CFG-3: Configuration Storage Format
**Covers**: REQ-CFG-3, REQ-CFG-4  
**Type**: Data Structure  

```c
// NVS storage layout
typedef struct {
    // Metadata (16 bytes)
    uint32_t magic_number;        // 0xCFG12345 - corruption detection
    uint32_t config_version;      // Schema version for migration
    uint32_t crc32_checksum;      // Data integrity verification  
    uint32_t save_count;          // Change tracking
    
    // Distance sensor configuration (20 bytes)
    float distance_min_cm;        // 4 bytes
    float distance_max_cm;        // 4 bytes  
    uint16_t measurement_interval_ms; // 2 bytes
    uint32_t sensor_timeout_ms;   // 4 bytes
    float smoothing_alpha;        // 4 bytes
    uint16_t reserved1;           // 2 bytes - future expansion
    
    // LED configuration (8 bytes)
    uint8_t led_count;            // 1 byte
    uint8_t led_brightness;       // 1 byte
    uint8_t led_rmt_channel;      // 1 byte
    uint8_t reserved2[5];         // 5 bytes - future expansion
    
    // WiFi configuration (12 bytes)
    uint8_t wifi_max_retry;       // 1 byte
    uint32_t wifi_timeout_ms;     // 4 bytes
    uint8_t wifi_ap_channel;      // 1 byte
    uint8_t wifi_ap_max_conn;     // 1 byte
    uint8_t reserved3[5];         // 5 bytes - future expansion
    
    // Web server configuration (8 bytes)
    uint32_t monitor_interval_ms; // 4 bytes
    uint16_t http_port;           // 2 bytes
    uint8_t reserved4[2];         // 2 bytes - future expansion
    
} __attribute__((packed)) config_nvs_storage_t; // Total: 64 bytes
```

**Storage Optimization**:

- **Fixed Size**: 64-byte structure for efficient NVS operations
- **Alignment**: Packed structure to minimize flash usage
- **Reserved Fields**: Expansion capability without breaking compatibility
- **Checksum**: CRC32 for corruption detection

## API Design

### DSN-CFG-4: Configuration API Implementation
**Covers**: REQ-CFG-5, REQ-CFG-6  
**Type**: Interface Design  

```c
// Public API header (main/config.h)
#ifndef CONFIG_H
#define CONFIG_H

#include "esp_err.h"
#include <stdbool.h>

// Configuration value access macros
#define CONFIG_GET_DISTANCE_MIN()     config_get_distance_min_cm()
#define CONFIG_GET_DISTANCE_MAX()     config_get_distance_max_cm()
#define CONFIG_GET_LED_COUNT()        config_get_led_count()
#define CONFIG_GET_LED_BRIGHTNESS()   config_get_led_brightness()

// Configuration management API
esp_err_t config_init(void);
esp_err_t config_load_from_nvs(void);
esp_err_t config_save_to_nvs(void);
esp_err_t config_factory_reset(void);

// Configuration value getters (thread-safe)
float config_get_distance_min_cm(void);
float config_get_distance_max_cm(void);
uint16_t config_get_measurement_interval_ms(void);
uint32_t config_get_sensor_timeout_ms(void);
float config_get_smoothing_alpha(void);
uint8_t config_get_led_count(void);
uint8_t config_get_led_brightness(void);

// Configuration value setters (with validation)
esp_err_t config_set_distance_range(float min_cm, float max_cm);
esp_err_t config_set_measurement_interval(uint16_t interval_ms);
esp_err_t config_set_sensor_timeout(uint32_t timeout_ms);
esp_err_t config_set_smoothing_alpha(float alpha);
esp_err_t config_set_led_count(uint8_t count);
esp_err_t config_set_led_brightness(uint8_t brightness);

// Bulk configuration operations
esp_err_t config_get_all(system_config_t* config);
esp_err_t config_set_all(const system_config_t* config);
bool config_validate_all(const system_config_t* config, char* error_msg, size_t error_msg_len);

#endif // CONFIG_H
```

**Thread Safety Design**:

- **Mutex Protection**: All configuration access protected by FreeRTOS mutex
- **Read-Write Lock**: Multiple readers, single writer pattern
- **Atomic Updates**: Configuration changes applied atomically
- **Timeout Handling**: API calls include timeout for responsiveness

## Web Interface Design

### DSN-CFG-5: Web Configuration Interface
**Covers**: REQ-CFG-7, REQ-CFG-8, REQ-CFG-9  
**Type**: User Interface Design  

**REST API Endpoints**:

```c
// Configuration REST API
GET  /api/config           -> Current configuration (JSON)
POST /api/config           -> Update configuration (JSON)
POST /api/config/preview   -> Temporary configuration preview
POST /api/config/apply     -> Apply previewed configuration
POST /api/config/reset     -> Factory reset
GET  /api/config/export    -> Export configuration (JSON download)
POST /api/config/import    -> Import configuration (JSON upload)
```

**Web Page Structure**:

```html
<!-- /settings page structure -->
<form id="config-form">
  <fieldset>
    <legend>Distance Sensor Settings</legend>
    <input type="range" id="distance-min" min="5" max="100" step="0.1">
    <input type="range" id="distance-max" min="20" max="400" step="0.1">
    <input type="range" id="measurement-interval" min="50" max="1000" step="10">
  </fieldset>
  
  <fieldset>
    <legend>LED Settings</legend>  
    <input type="range" id="led-count" min="1" max="60" step="1">
    <input type="range" id="led-brightness" min="10" max="255" step="1">
  </fieldset>
  
  <div class="config-actions">
    <button type="button" id="preview-btn">Preview Changes</button>
    <button type="button" id="apply-btn">Apply & Save</button>
    <button type="button" id="reset-btn">Factory Reset</button>
  </div>
</form>
```

**Real-time Preview Logic**:

1. **Preview Mode**: POST to `/api/config/preview` applies temporary changes
2. **Visual Feedback**: LED brightness changes visible immediately  
3. **Auto-Revert**: Preview timeout (30 seconds) reverts automatically
4. **Apply Confirmation**: User must explicitly apply changes permanently

## Implementation Strategy

### DSN-CFG-6: GitHub Copilot Implementation Plan
**Covers**: All REQ-CFG requirements  
**Type**: Implementation Strategy  

**Phase 1: AI-Assisted Core Implementation**

GitHub Copilot will be assigned to implement:

```
1. Magic Number Consolidation (REQ-CFG-1, REQ-CFG-2)
   - Create main/config.h with all centralized values
   - Update all source files to use config.h macros
   - Validate no magic numbers remain in codebase

2. Configuration Data Structures (REQ-CFG-3)  
   - Implement system_config_t structure
   - Add validation ranges and metadata
   - Create NVS storage layout

3. Configuration API (REQ-CFG-4, REQ-CFG-5, REQ-CFG-6)
   - Implement thread-safe configuration access
   - Add NVS persistence layer
   - Implement parameter validation
```

**Phase 2: Human-AI Collaboration**

```
1. Web Interface (REQ-CFG-7, REQ-CFG-8)
   - AI: Generate REST API endpoints and handlers
   - Human: Review security and user experience
   - AI: Implement real-time preview functionality

2. Advanced Features (REQ-CFG-9, REQ-CFG-10, REQ-CFG-11) 
   - AI: Implement backup/restore functionality
   - Human: Validate performance requirements
   - AI: Add comprehensive error handling
```

**Quality Assurance Process**:

1. **Requirement Validation**: Each implementation verified against acceptance criteria
2. **Integration Testing**: Configuration system tested with existing components  
3. **Performance Testing**: Configuration operations measured for real-time compliance
4. **Security Review**: Web interface reviewed for security vulnerabilities

---

*This document follows OpenFastTrack methodology for design traceability.*
