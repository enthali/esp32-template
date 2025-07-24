# Configuration Management Test Specification

**Document ID**: TST-CFG-ESP32-DISTANCE  
**Version**: 1.0  
**Date**: 2025-07-24  
**Author**: ESP32 Distance Project Team  
**Requirements Traceability**: REQ-CFG-1 through REQ-CFG-11  
**Design Traceability**: DSN-CFG-1 through DSN-CFG-6  

## Test Overview

This document specifies test cases for the Configuration Management System, ensuring compliance with requirements and design specifications.

## Test Categories

- **Unit Tests**: Individual function validation
- **Integration Tests**: Component interaction validation  
- **System Tests**: End-to-end functionality validation
- **Performance Tests**: Real-time requirements validation
- **Reliability Tests**: Error handling and recovery validation

---

## Phase 1: Magic Number Consolidation Tests

### TST-CFG-1: Configuration Header Validation

**Covers**: REQ-CFG-1, DSN-CFG-1  
**Type**: Unit Test  
**Priority**: Mandatory  

**Test Objective**: Verify all magic numbers consolidated into config.h

**Test Cases**:

#### TST-CFG-1.1: Configuration Header Completeness

**Test Steps**:

1. Parse config.h and extract all #define statements
2. Verify distance sensor configuration constants present
3. Verify LED controller configuration constants present  
4. Verify WiFi configuration constants present
5. Verify web server configuration constants present
6. Verify all constants include documentation comments

**Expected Results**:

- All configuration categories represented in config.h
- Each constant includes value and comment explaining purpose
- Constants follow naming convention:

``` HTML
DEFAULT_<CATEGORY>_<NAME>
```

#### TST-CFG-1.2: Source Code Magic Number Elimination

**Test Steps**:

1. Scan all .c files in main/ directory for numeric literals
2. Scan all .c files in components/ directory for numeric literals
3. Exclude acceptable literals (0, 1, NULL, array indices)
4. Verify remaining literals reference config.h constants

**Expected Results**:

- No magic numbers found in main/main.c
- No magic numbers found in main/wifi_manager.c  
- No magic numbers found in main/web_server.c
- No magic numbers found in components/distance_sensor/
- No magic numbers found in components/led_controller/

### TST-CFG-2: Source Code Migration Validation

**Covers**: REQ-CFG-2, DSN-CFG-1  
**Type**: Integration Test  
**Priority**: Mandatory  

**Test Objective**: Verify source files correctly use centralized configuration

#### TST-CFG-2.1: Build System Validation

**Test Steps**:

1. Build project with config.h modifications
2. Verify all source files compile without errors
3. Verify all config.h constants resolved at compile time
4. Check for unused configuration constants

**Expected Results**:

- Clean build with no compilation warnings
- All configuration constants referenced in code
- No undefined references to configuration values

---

## Phase 2: Runtime Configuration Tests

### TST-CFG-3: Configuration Data Structure Tests

**Covers**: REQ-CFG-3, DSN-CFG-3  
**Type**: Unit Test  
**Priority**: Mandatory  

#### TST-CFG-3.1: Structure Size Validation

**Test Steps**:

1. Verify sizeof(config_nvs_storage_t) equals 64 bytes
2. Verify structure alignment and padding
3. Verify all fields accessible at expected offsets

**Expected Results**:

- Structure size exactly 64 bytes for NVS efficiency
- No unexpected padding between fields
- All fields aligned properly for ESP32 architecture

#### TST-CFG-3.2: Default Value Consistency

**Test Steps**:

1. Initialize system_config_t with default values
2. Compare with compile-time constants in config.h
3. Verify all default values within specified ranges

**Expected Results**:

- Runtime defaults match compile-time constants
- All default values pass validation checks
- No inconsistencies between config.h and runtime structure

### TST-CFG-4: NVS Storage Tests  

**Covers**: REQ-CFG-4, DSN-CFG-2  
**Type**: Integration Test  
**Priority**: Mandatory  

#### TST-CFG-4.1: NVS Persistence Test

**Test Steps**:

1. Save configuration to NVS
2. Reset ESP32 (software reset)
3. Load configuration from NVS
4. Verify all values match saved configuration

**Expected Results**:

- Configuration survives device reset
- All parameter values preserved exactly
- No data corruption during power cycle

#### TST-CFG-4.2: NVS Corruption Handling

**Test Steps**:

1. Corrupt NVS configuration data (modify checksum)
2. Attempt to load configuration
3. Verify error detection and fallback to defaults
4. Verify system continues normal operation

**Expected Results**:

- Corruption detected via checksum mismatch
- Automatic fallback to default configuration
- Error logged for diagnostic purposes
- System remains operational with defaults

### TST-CFG-5: Configuration API Tests

**Covers**: REQ-CFG-5, REQ-CFG-6, DSN-CFG-4  
**Type**: Unit Test  
**Priority**: Mandatory  

#### TST-CFG-5.1: API Function Validation

**Test Steps**:

1. Test config_init() return value and initialization
2. Test config_load() with valid and invalid NVS data
3. Test config_save() with valid and invalid parameters
4. Test config_factory_reset() operation
5. Test all getter functions for thread safety
6. Test all setter functions with range validation

**Expected Results**:

- All API functions return appropriate esp_err_t codes
- Invalid parameters rejected with ESP_ERR_INVALID_ARG
- Thread-safe access verified under concurrent load
- Range validation prevents invalid configurations

#### TST-CFG-5.2: Parameter Validation Tests

**Test Steps**:

1. Test distance range validation (min < max)
2. Test parameter boundary conditions (min, max, invalid)
3. Test inter-parameter validation logic
4. Test validation error message generation

**Expected Results**:

- Boundary values (5.0, 100.0 for distance_min) accepted
- Out-of-range values (4.9, 100.1) rejected  
- Invalid combinations (min > max) rejected
- Clear error messages generated for invalid inputs

---

## Phase 3: Web Interface Tests

### TST-CFG-6: Web Configuration Interface Tests

**Covers**: REQ-CFG-7, REQ-CFG-8, DSN-CFG-5  
**Type**: System Test  
**Priority**: High  

#### TST-CFG-6.1: Web Settings Page Test

**Test Steps**:

1. Navigate to /settings page in web browser
2. Verify all configuration parameters displayed
3. Verify current values pre-populated in form fields
4. Test form submission with valid parameters
5. Test form submission with invalid parameters
6. Verify success/error feedback to user

**Expected Results**:

- Settings page loads without errors
- All runtime parameters accessible via web form
- Current configuration values displayed correctly
- Valid changes accepted and applied
- Invalid changes rejected with user feedback

#### TST-CFG-6.2: Real-time Preview Test

**Test Steps**:

1. Access settings page and modify LED brightness
2. Click "Preview" button
3. Verify LED strip brightness changes immediately
4. Wait for preview timeout (30 seconds)
5. Verify automatic revert to previous brightness
6. Test "Apply" button to make changes permanent

**Expected Results**:

- Preview changes visible immediately on LED strip
- Preview timeout reverts to saved configuration
- Apply button makes preview changes permanent
- Cancel button reverts to saved configuration

### TST-CFG-7: Configuration Backup/Restore Tests

**Covers**: REQ-CFG-9, DSN-CFG-5  
**Type**: System Test  
**Priority**: Low  

#### TST-CFG-7.1: Configuration Export Test

**Test Steps**:

1. Configure system with non-default values
2. Click "Export Configuration" button
3. Verify JSON file downloaded
4. Parse JSON and validate configuration data
5. Verify JSON includes metadata (version, timestamp)

**Expected Results**:

- JSON file downloads successfully
- JSON contains all configuration parameters
- Configuration values match current system settings
- Metadata included for version tracking

#### TST-CFG-7.2: Configuration Import Test

**Test Steps**:

1. Export current configuration as baseline
2. Modify configuration via settings page
3. Import baseline configuration JSON file
4. Verify configuration restored to baseline values
5. Test import with invalid JSON (should reject)

**Expected Results**:

- Valid JSON import restores configuration correctly
- Invalid JSON rejected with error message
- Import operation atomic (success or no change)
- Configuration validation applied during import

---

## Performance Tests

### TST-CFG-8: Configuration Performance Tests

**Covers**: REQ-CFG-10, DSN-CFG-4  
**Type**: Performance Test  
**Priority**: Mandatory  

#### TST-CFG-8.1: Startup Performance Test

**Test Steps**:

1. Measure time from system boot to config_init() completion
2. Measure time for config_load_from_nvs() operation
3. Verify total configuration loading time
4. Test under various NVS data conditions

**Expected Results**:

- Configuration initialization completes within 100ms
- NVS load operation completes within 50ms
- Performance consistent across multiple test runs
- No performance degradation with configuration size

#### TST-CFG-8.2: Runtime Performance Test

**Test Steps**:

1. Measure config_get_*() function call latency
2. Measure config_save_to_nvs() operation time
3. Test concurrent access performance
4. Verify no impact on sensor measurement timing

**Expected Results**:

- Configuration getter functions complete within 1ms
- Configuration save operations complete within 500ms
- Concurrent access does not cause blocking
- Sensor measurements maintain 10Hz frequency during config operations

---

## Reliability Tests

### TST-CFG-9: Configuration Reliability Tests

**Covers**: REQ-CFG-11, DSN-CFG-2  
**Type**: Reliability Test  
**Priority**: Mandatory  

#### TST-CFG-9.1: Power Loss Simulation

**Test Steps**:

1. Initiate configuration save operation
2. Simulate power loss at various points during save
3. Restart system and attempt configuration load
4. Verify NVS integrity and error handling

**Expected Results**:

- NVS corruption detected if power lost during write
- System recovers gracefully with default configuration
- No system crash or hang due to NVS corruption
- Error logged for diagnostic purposes

#### TST-CFG-9.2: Network Disconnection Test

**Test Steps**:

1. Access web configuration interface
2. Disconnect WiFi network during configuration update
3. Verify web interface handles disconnection gracefully
4. Reconnect and verify configuration state

**Expected Results**:

- Web interface detects network disconnection
- Configuration changes not lost during network interruption
- Graceful error handling without system crash
- Configuration state consistent after reconnection

---

## Test Automation

### GitHub Copilot Test Implementation

**Implementation Scope**: Automated test suite generation using AI assistance

**Copilot Assignment**:

1. **Unit Test Generation**: Implement automated tests for all configuration API functions
2. **Integration Test Scripts**: Create test scripts for NVS operations and web interface
3. **Performance Benchmarks**: Generate performance measurement code
4. **Test Data Generation**: Create test data sets for boundary condition testing

**Human Validation**:

- Review test coverage completeness
- Validate test case correctness
- Approve test automation framework
- Verify performance benchmark accuracy

---

*This document follows OpenFastTrack methodology for test traceability.*
