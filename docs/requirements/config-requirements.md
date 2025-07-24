# Configuration Management Requirements

**Document ID**: REQ-CFG-ESP32-DISTANCE  
**Version**: 1.0  
**Date**: 2025-07-24  
**Author**: ESP32 Distance Project Team  
**Parent Document**: SRS-ESP32-DISTANCE  

## Document Purpose

This document specifies detailed requirements for the Configuration Management System, enabling centralized parameter management, runtime configuration, and persistent storage.

## Requirements Traceability

| Requirement ID | Parent Requirement | Design Reference | Test Reference |
|----------------|-------------------|------------------|----------------|
| REQ-CFG-1      | REQ-SYS-5        | DSN-CFG-1        | TST-CFG-1     |
| REQ-CFG-2      | REQ-SYS-5        | DSN-CFG-2        | TST-CFG-2     |
| REQ-CFG-3      | REQ-SYS-4        | DSN-CFG-3        | TST-CFG-3     |

---

## Phase 1: Magic Number Consolidation

### REQ-CFG-1: Centralized Configuration Header
**Type**: Implementation  
**Priority**: Mandatory  
**Description**: The system SHALL consolidate all hardcoded configuration values into a single header file `main/config.h`.

**Rationale**: Eliminates scattered magic numbers throughout the codebase, improving maintainability and reducing configuration errors.

**Acceptance Criteria**:

- AC-1: All distance sensor configuration values centralized in config.h
- AC-2: All LED controller configuration values centralized in config.h  
- AC-3: All WiFi configuration values centralized in config.h
- AC-4: All web server configuration values centralized in config.h
- AC-5: All system timing configuration values centralized in config.h
- AC-6: No magic numbers remain in source files outside config.h
- AC-7: Each configuration value documented with purpose and valid range

**Configuration Categories**:

```c
// Distance Sensor Configuration
#define DEFAULT_DISTANCE_MIN_CM         10.0f    // Range: 5.0-100.0
#define DEFAULT_DISTANCE_MAX_CM         50.0f    // Range: 20.0-400.0  
#define DEFAULT_DISTANCE_INTERVAL_MS    100      // Range: 50-1000
#define DEFAULT_DISTANCE_TIMEOUT_MS     30       // Range: 10-100
#define DEFAULT_TEMPERATURE_C           20.0f    // Range: -20.0-60.0
#define DEFAULT_SMOOTHING_ALPHA         0.3f     // Range: 0.1-1.0

// LED Controller Configuration  
#define DEFAULT_LED_COUNT               40       // Range: 1-60
#define DEFAULT_LED_BRIGHTNESS          128      // Range: 10-255
#define DEFAULT_LED_RMT_CHANNEL         0        // Range: 0-7

// WiFi Configuration
#define DEFAULT_WIFI_AP_CHANNEL         1        // Range: 1-13
#define DEFAULT_WIFI_AP_MAX_CONN        4        // Range: 1-10
#define DEFAULT_WIFI_STA_MAX_RETRY      3        // Range: 1-10
#define DEFAULT_WIFI_STA_TIMEOUT_MS     5000     // Range: 1000-30000

// Web Server Configuration
#define DEFAULT_HTTP_PORT               80       // Range: 1-65535
#define DEFAULT_MAX_URI_LENGTH          64       // Range: 32-256
```

### REQ-CFG-2: Source Code Migration
**Type**: Implementation  
**Priority**: Mandatory  
**Description**: All source files SHALL be updated to reference centralized configuration values instead of local magic numbers.

**Rationale**: Ensures consistent configuration usage across the entire codebase.

**Acceptance Criteria**:

- AC-1: main/main.c updated to use config.h values
- AC-2: main/wifi_manager.c updated to use config.h values
- AC-3: main/web_server.c updated to use config.h values
- AC-4: components/distance_sensor/ updated to use config.h values
- AC-5: components/led_controller/ updated to use config.h values
- AC-6: Build system validates no magic numbers in source files
- AC-7: Code review confirms all references point to config.h

---

## Phase 2: Runtime Configuration System

### REQ-CFG-3: Configuration Data Structure
**Type**: Design  
**Priority**: Mandatory  
**Description**: The system SHALL define a runtime configuration structure with validation ranges for all user-configurable parameters.

**Rationale**: Enables runtime parameter modification while ensuring system stability through validation.

**Acceptance Criteria**:

- AC-1: Configuration structure includes all runtime-modifiable parameters
- AC-2: Each parameter includes minimum and maximum valid ranges  
- AC-3: Configuration structure includes versioning for compatibility
- AC-4: Structure optimized for NVS storage efficiency
- AC-5: Default values match compile-time constants in config.h

**Configuration Structure**:

```c
typedef struct {
    // Configuration metadata
    uint32_t config_version;         // Current: 1
    uint32_t save_count;             // Change tracking
    
    // Distance sensor settings (runtime configurable)
    float distance_min_cm;           // Range: 5.0 - 100.0
    float distance_max_cm;           // Range: 20.0 - 400.0
    uint16_t measurement_interval_ms; // Range: 50 - 1000
    uint32_t sensor_timeout_ms;      // Range: 10 - 100
    float smoothing_alpha;           // Range: 0.1 - 1.0
    
    // LED settings (runtime configurable)
    uint8_t led_count;               // Range: 1 - 60
    uint8_t led_brightness;          // Range: 10 - 255
    
    // WiFi settings (runtime configurable)
    uint8_t wifi_max_retry;          // Range: 1 - 10
    uint32_t wifi_timeout_ms;        // Range: 1000 - 30000
    
    // Web server settings (runtime configurable)
    uint32_t monitor_interval_ms;    // Range: 1000 - 60000
} system_config_t;
```

### REQ-CFG-4: Non-Volatile Storage (NVS)
**Type**: Implementation  
**Priority**: Mandatory  
**Description**: The system SHALL store runtime configuration in ESP32 NVS flash memory with persistence across power cycles.

**Rationale**: Maintains user configuration permanently without requiring firmware recompilation.

**Acceptance Criteria**:

- AC-1: Configuration saved to NVS namespace "esp32_distance_config"
- AC-2: Configuration survives device reset and power loss
- AC-3: NVS corruption detection and error handling
- AC-4: Automatic fallback to defaults if NVS read fails
- AC-5: NVS write operations protected against power loss
- AC-6: Configuration integrity verified with checksum

### REQ-CFG-5: Configuration API
**Type**: Implementation  
**Priority**: Mandatory  
**Description**: The system SHALL provide a well-defined API for configuration management operations.

**Rationale**: Enables consistent configuration access across all system components.

**Acceptance Criteria**:

- AC-1: config_init() initializes configuration subsystem
- AC-2: config_load() reads configuration from NVS with error handling
- AC-3: config_save() writes configuration to NVS with validation
- AC-4: config_validate_range() validates all parameter ranges
- AC-5: config_factory_reset() restores compile-time defaults
- AC-6: All API functions return appropriate esp_err_t codes
- AC-7: Thread-safe access with mutex protection

**API Specification**:

```c
// Configuration management API
esp_err_t config_init(void);
esp_err_t config_load(system_config_t* config);
esp_err_t config_save(const system_config_t* config);
esp_err_t config_validate_range(const system_config_t* config);
esp_err_t config_factory_reset(void);
bool config_is_valid_range(const char* param_name, float value);
```

### REQ-CFG-6: Parameter Validation
**Type**: Implementation  
**Priority**: Mandatory  
**Description**: The system SHALL validate all configuration parameters against defined ranges before acceptance.

**Rationale**: Prevents invalid configurations that could cause system malfunction or instability.

**Acceptance Criteria**:

- AC-1: All float parameters validated against min/max ranges
- AC-2: All integer parameters validated against min/max ranges  
- AC-3: Inter-parameter validation (e.g., min_distance < max_distance)
- AC-4: Invalid parameters rejected with specific error messages
- AC-5: Validation performed before NVS save operations
- AC-6: Validation errors logged with parameter name and attempted value

---

## Phase 3: Web Configuration Interface

### REQ-CFG-7: Web Settings Page
**Type**: Implementation  
**Priority**: High  
**Description**: The system SHALL provide a web interface for runtime configuration modification.

**Rationale**: Enables user-friendly configuration without requiring technical knowledge or firmware recompilation.

**Acceptance Criteria**:

- AC-1: Web page accessible at `/settings` endpoint
- AC-2: Form fields for all runtime-configurable parameters
- AC-3: Current values pre-populated in form fields
- AC-4: Parameter ranges displayed as input constraints
- AC-5: Client-side validation before form submission
- AC-6: Server-side validation with error feedback
- AC-7: Success confirmation after configuration save

### REQ-CFG-8: Real-time Configuration Preview
**Type**: Implementation  
**Priority**: Medium  
**Description**: The system SHALL provide real-time preview of configuration changes before permanent application.

**Rationale**: Allows users to test configuration changes safely before committing them permanently.

**Acceptance Criteria**:

- AC-1: "Preview" mode applies changes temporarily (RAM only)
- AC-2: LED brightness changes visible immediately in preview
- AC-3: Distance range changes visible in LED mapping preview
- AC-4: Preview timeout reverts to previous configuration automatically
- AC-5: "Apply" button makes preview changes permanent
- AC-6: "Cancel" button reverts to saved configuration

### REQ-CFG-9: Configuration Backup and Restore
**Type**: Implementation  
**Priority**: Low  
**Description**: The system SHALL support configuration backup and restore via JSON export/import.

**Rationale**: Enables configuration transfer between devices and recovery from misconfiguration.

**Acceptance Criteria**:

- AC-1: "Export" function downloads configuration as JSON file
- AC-2: "Import" function accepts JSON configuration file upload
- AC-3: Import validation ensures JSON format and parameter ranges
- AC-4: Import operation preserves existing configuration on validation failure
- AC-5: Exported JSON includes configuration metadata (version, timestamp)

---

## Quality Requirements

### REQ-CFG-10: Configuration Performance
**Type**: Performance  
**Priority**: Mandatory  
**Description**: Configuration operations SHALL not impact real-time system performance.

**Acceptance Criteria**:

- AC-1: Configuration load during startup completes within 100ms
- AC-2: Configuration save operations complete within 500ms
- AC-3: Configuration changes applied without sensor measurement interruption
- AC-4: Web configuration interface remains responsive during operations

### REQ-CFG-11: Configuration Reliability
**Type**: Reliability  
**Priority**: Mandatory  
**Description**: Configuration system SHALL handle all error conditions gracefully.

**Acceptance Criteria**:

- AC-1: NVS corruption detected and reported with recovery action
- AC-2: Invalid parameter ranges rejected with specific error messages
- AC-3: Power loss during configuration save does not corrupt NVS
- AC-4: Network disconnection during web configuration handled gracefully
- AC-5: System operates with default configuration if NVS unavailable

---

## Implementation Assignment

**GitHub Copilot Implementation Scope**: This requirement specification is designated for AI-assisted implementation using GitHub Copilot.

**Copilot Assignment Areas**:

1. **Phase 1**: Magic number consolidation and source code migration
2. **Phase 2**: NVS storage implementation and configuration API
3. **Phase 3**: Web interface development and real-time preview

**Human Review Points**:

- Requirement compliance verification
- Security review of web interface
- Integration testing with existing components
- Performance validation under load

---

*This document follows OpenFastTrack methodology for requirements engineering and traceability.*
