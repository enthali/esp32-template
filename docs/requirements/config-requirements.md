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

**Dependencies**:
- REQ-CFG-2 depends on REQ-CFG-1 (cannot use centralized configuration that doesn't exist)
- REQ-CFG-3 depends on REQ-CFG-1 (runtime structure must match compile-time constants)
- REQ-CFG-4 depends on REQ-CFG-3 (cannot store configuration structure that doesn't exist)
- REQ-CFG-5 depends on REQ-CFG-4 (cannot implement NVS API without NVS storage requirement)
- REQ-CFG-6 depends on REQ-CFG-3 (cannot validate parameters without defined structure)

---

## Static Configuration Management

### REQ-CFG-1: Centralized Configuration Header

**Type**: Implementation  
**Priority**: Mandatory  
**Description**: The system SHALL consolidate all user-configurable parameters and system-level configuration values, defined in this requirement, into a single header file `main/config.h`, while hardware-specific constants and protocol specifications remain in their respective component files.

**Rationale**: Eliminates scattered magic numbers for configurable parameters throughout the codebase, improving maintainability and reducing configuration errors, while preserving component encapsulation for hardware-specific values.

**Acceptance Criteria**:

- AC-1: Distance sensor parameters (DEFAULT_DISTANCE_MIN_CM, DEFAULT_DISTANCE_MAX_CM, DEFAULT_MEASUREMENT_INTERVAL_MS, DEFAULT_SENSOR_TIMEOUT_MS, DEFAULT_TEMPERATURE_C, DEFAULT_SMOOTHING_ALPHA) centralized in config.h
- AC-2: LED controller parameters (DEFAULT_LED_COUNT, DEFAULT_LED_BRIGHTNESS) centralized in config.h  
- AC-3: WiFi parameters (DEFAULT_WIFI_AP_CHANNEL, DEFAULT_WIFI_AP_MAX_CONN, DEFAULT_WIFI_STA_MAX_RETRY, DEFAULT_WIFI_STA_TIMEOUT_MS) centralized in config.h
- AC-4: All parameters defined in the Configuration Categories section below SHALL be in config.h
- AC-5: No additional user-configurable magic numbers remain in source files outside config.h
- AC-6: Each configuration value documented with purpose and valid range as shown in Configuration Categories

**Configuration Categories**:

```c
// Distance Sensor Configuration (User Configurable)
#define DEFAULT_DISTANCE_MIN_CM         10.0f    // The distance mapped to the first LED : Range: 5.0-100.0 
#define DEFAULT_DISTANCE_MAX_CM         50.0f    // The distance mapped to the last LED, must be larger than DEFAULT_DISTANCE_MIN_CM : Range: 20.0-400.0  
#define DEFAULT_MEASUREMENT_INTERVAL_MS 100      // How often to measure distance in milliseconds : Range: 50-1000
#define DEFAULT_SENSOR_TIMEOUT_MS       30       // Maximum time to wait for ultrasonic echo, must be < interval : Range: 10-50
#define DEFAULT_TEMPERATURE_C           20.0f    // Ambient temperature for sound speed calculation : Range: -20.0-60.0
#define DEFAULT_SMOOTHING_ALPHA         0.3f     // Exponential moving average smoothing factor : Range: 0.1-1.0

// LED Controller Configuration (User Configurable)
#define DEFAULT_LED_COUNT               40       // Number of LEDs in the strip : Range: 1-60
#define DEFAULT_LED_BRIGHTNESS          128      // LED brightness level (0=off, 255=max) : Range: 10-255

// WiFi Configuration (User Configurable)
#define DEFAULT_WIFI_AP_CHANNEL         1        // WiFi access point channel : Range: 1-13
#define DEFAULT_WIFI_AP_MAX_CONN        2        // Maximum simultaneous AP connections : Range: 1-10
#define DEFAULT_WIFI_STA_MAX_RETRY      3        // Station connection retry attempts : Range: 1-10
#define DEFAULT_WIFI_STA_TIMEOUT_MS     5000     // Station connection timeout : Range: 1000-30000
```

**Scope Definition**:

- **Included in config.h**: User-configurable parameters, system behavior settings, timing intervals, network parameters
- **Excluded from config.h**: Hardware timing specifications (WS2812 bit timing), ESP-IDF task stack sizes, protocol constants (HTTP status codes), component-internal buffer sizes

**Note**: This selective approach maintains component encapsulation while centralizing parameters that affect system behavior or user experience.

### REQ-CFG-2: Use of Centralized Configuration

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-CFG-1  
**Description**: All system modules SHALL use centralized configuration values defined in REQ-CFG-1 for parameters specified in the centralized configuration header, while retaining module-specific constants for hardware and protocol specifications.

**Rationale**: Ensures consistent use of configurable parameters across the system while allowing modules to maintain their own module-specific constants.

**Acceptance Criteria**:

- AC-1: All modules SHALL reference config.h for parameters defined therein
- AC-2: Modules SHALL NOT define local copies of centrally-defined configuration values
- AC-3: Modules MAY retain hardware-specific constants (WS2812 timing, task stack sizes, etc.)
- AC-4: Modules MAY retain protocol-specific constants (HTTP status codes, buffer sizes, etc.)
- AC-5: Build process SHALL validate no duplicate definitions of centralized parameters
- AC-6: Each centralized parameter SHALL be used consistently across all referencing modules
- AC-7: Module documentation SHALL clearly distinguish between centralized and local parameters

**Scope Clarification**:

- **Centralized Parameters**: All user-configurable values defined in config.h (distance ranges, LED settings, WiFi parameters)
- **Local Parameters**: Hardware timings, protocol constants, module-internal buffer sizes, task priorities
- **Example**: `DEFAULT_LED_COUNT` must come from config.h, but `WS2812_T0H_NS` remains in led_controller component

---

## Dynamic Configuration Management

### REQ-CFG-3: Configuration Data Structure

**Type**: Design  
**Priority**: Mandatory  
**Depends**: REQ-CFG-1  
**Description**: The system SHALL define a runtime configuration structure containing all user-configurable parameters defined in REQ-CFG-1, optimized for NVS storage and runtime modification.

**Rationale**: Enables runtime parameter modification while maintaining consistency with compile-time defaults and ensuring efficient storage in ESP32 NVS flash memory.

**Acceptance Criteria**:

- AC-1: Configuration structure includes all runtime-modifiable parameters from REQ-CFG-1
- AC-2: Structure includes metadata for versioning and change tracking
- AC-3: Data types optimized for NVS storage efficiency (uint8_t, uint16_t, uint32_t, float)
- AC-4: String fields sized appropriately (WiFi SSID: 33 chars, password: 65 chars)
- AC-5: Default values match compile-time constants defined in REQ-CFG-1
- AC-6: Structure layout compatible with ESP32 memory alignment requirements

**Configuration Structure**:

```c
typedef struct {
    // Configuration metadata
    uint32_t config_version;         // Current: 1
    uint32_t save_count;             // Change tracking
    
    // Distance sensor settings (runtime configurable)
    float distance_min_cm;           
    float distance_max_cm;           
    uint16_t measurement_interval_ms; 
    uint32_t sensor_timeout_ms;      
    float temperature_c;             
    float smoothing_alpha;           
    
    // LED settings (runtime configurable)
    uint8_t led_count;               
    uint8_t led_brightness;          
    
    // WiFi settings (runtime configurable)
    char wifi_ssid[33];              // WiFi network name (null-terminated, max 32 chars)
    char wifi_password[65];          // WiFi network password (null-terminated, max 64 chars)
    uint8_t wifi_ap_channel;         
    uint8_t wifi_ap_max_conn;        
    uint8_t wifi_sta_max_retry;      
    uint32_t wifi_sta_timeout_ms;    
} system_config_t;
```

### REQ-CFG-4: Non-Volatile Storage (NVS)

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-CFG-3  
**Description**: The system SHALL store runtime configuration defined in REQ-CFG-3 in ESP32 NVS flash memory with persistence across power cycles.

**Rationale**: Maintains user configuration permanently without requiring firmware recompilation, using the configuration structure defined in REQ-CFG-3.

**Acceptance Criteria**:

- AC-1: Configuration saved to NVS namespace "esp32_distance_config"
- AC-2: Configuration survives device reset and power loss
- AC-3: NVS write operations are atomic and protected against power loss
- AC-4: Configuration integrity maintained through power cycles

### REQ-CFG-5: Configuration API

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-CFG-4  
**Description**: The system SHALL provide a well-defined API for configuration management operations with robust error handling and automatic fallback to defaults.

**Rationale**: Enables consistent configuration access across all system components while handling NVS storage errors gracefully.

**Acceptance Criteria**:

- AC-1: config_init() initializes configuration subsystem
- AC-2: config_load() reads configuration from NVS with error handling
- AC-3: config_save() writes configuration to NVS with validation
- AC-4: config_validate_range() validates all parameter ranges
- AC-5: config_factory_reset() restores compile-time defaults from REQ-CFG-1
- AC-6: All API functions return appropriate esp_err_t codes
- AC-7: Thread-safe access with mutex protection
- AC-8: NVS corruption detection and error handling
- AC-9: Automatic fallback to defaults if NVS read fails
- AC-10: Configuration integrity verified with checksum

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
**Depends**: REQ-CFG-3  
**Description**: The system SHALL validate all configuration parameters in the runtime configuration structure against defined ranges before acceptance.

**Rationale**: Prevents invalid configurations that could cause system malfunction or instability by enforcing parameter bounds and inter-parameter relationships.

**Acceptance Criteria**:

- AC-1: All float parameters validated against min/max ranges as specified below
- AC-2: All integer parameters validated against min/max ranges as specified below
- AC-3: Inter-parameter validation (e.g., distance_min_cm < distance_max_cm, sensor_timeout_ms < measurement_interval_ms)
- AC-4: Invalid parameters rejected with specific error messages
- AC-5: Validation performed before NVS save operations
- AC-6: Validation errors logged with parameter name and attempted value

**Parameter Validation Ranges**:

```c
// Distance sensor parameter ranges
distance_min_cm:           5.0 - 100.0
distance_max_cm:           20.0 - 400.0 (must be > distance_min_cm)
measurement_interval_ms:   50 - 1000
sensor_timeout_ms:         10 - 50 (must be < measurement_interval_ms)
temperature_c:             -20.0 - 60.0
smoothing_alpha:           0.1 - 1.0

// LED parameter ranges
led_count:                 1 - 60
led_brightness:            10 - 255

// WiFi parameter ranges
wifi_ap_channel:           1 - 13
wifi_ap_max_conn:          1 - 10
wifi_sta_max_retry:        1 - 10
wifi_sta_timeout_ms:       1000 - 30000
```

---

## Configuration User Interface

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

1. **Static Configuration**: Centralized parameter definition and module integration
   - **TASK-CFG-001**: Create centralized config.h header file
   - **TASK-CFG-002**: Source code migration to use config.h values
   - **TASK-CFG-002.1**: WiFi manager NVS migration (see Features-planned.md)

2. **Dynamic Configuration**: Runtime configuration system and persistence
   - **TASK-CFG-003**: Configuration data structure and API implementation
   - **TASK-CFG-004**: NVS storage with persistence and validation

3. **Configuration Interface**: User-facing configuration capabilities
   - **TASK-CFG-005**: Web settings page implementation
   - **TASK-CFG-006**: Real-time configuration preview system

**Human Review Points**:

- Requirement compliance verification
- Security review of web interface
- Integration testing with existing components
- Performance validation under load

---

*This document follows OpenFastTrack methodology for requirements engineering and traceability.*
