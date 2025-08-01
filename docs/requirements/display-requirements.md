# Display Logic Requirements

**Document ID**: REQ-DSP-ESP32-DISTANCE  
**Version**: 1.0  
**Date**: 2025-07-30  
**Author**: ESP32 Distance Project Team  
**Parent Document**: SRS-ESP32-DISTANCE  

## Document Purpose

This document specifies detailed requirements for the Display Logic System, enabling visual representation of distance measurements through LED strip control and real-time distance-to-LED mapping.

## Requirements Traceability

| Requirement ID | Parent Requirement | Design Reference | Test Reference |
|----------------|-------------------|------------------|----------------|
| REQ-DSP-1      | REQ-SYS-3        | DSN-DSP-1        | TST-DSP-1     |
| REQ-DSP-2      | REQ-SYS-3        | DSN-DSP-2        | TST-DSP-2     |
| REQ-DSP-3      | REQ-SYS-3        | DSN-DSP-3        | TST-DSP-3     |

**Dependencies**:
- REQ-DSP-2 depends on REQ-DSP-1 (cannot start display logic without initialization)
- REQ-DSP-3 depends on REQ-DSP-1 (cannot map distance without configuration)
- REQ-DSP-4 depends on REQ-DSP-3 (cannot update display without mapping logic)
- REQ-DSP-5 depends on REQ-DSP-2 (cannot process measurements without running task)

---

## Core Display Logic

### REQ-DSP-1: Display Logic Initialization

**Type**: Implementation  
**Priority**: Mandatory  
**Description**: The system SHALL provide initialization functionality for the display logic subsystem with configurable distance-to-LED mapping parameters.

**Rationale**: Enables flexible configuration of the distance visualization range and validates system dependencies before starting the display logic task.

**Acceptance Criteria**:

- AC-1: display_logic_init() function accepts display_config_t configuration parameter
- AC-2: Configuration validation ensures min_distance_cm < max_distance_cm
- AC-3: Initialization verifies LED controller is initialized before proceeding
- AC-4: LED count verification warns if not exactly 40 LEDs as expected
- AC-5: Initialization can only be called once (subsequent calls return ESP_ERR_INVALID_STATE)
- AC-6: Invalid configuration parameters return ESP_ERR_INVALID_ARG
- AC-7: Configuration is stored internally for use by mapping functions

**Configuration Structure**:

```c
typedef struct {
    float min_distance_cm;    // Minimum distance mapped to LED 0
    float max_distance_cm;    // Maximum distance mapped to LED 39
} display_config_t;
```

### REQ-DSP-2: Display Logic Task Management

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-DSP-1  
**Description**: The system SHALL provide task lifecycle management for the display logic with start, stop, and status functions.

**Rationale**: Enables controlled execution of the display logic as a FreeRTOS task with proper resource management and state tracking.

**Acceptance Criteria**:

- AC-1: display_logic_start() creates FreeRTOS task with priority 3 on core 1
- AC-2: Task creation requires prior initialization (returns ESP_ERR_INVALID_STATE if not initialized)
- AC-3: Task stack size configured to 4KB for adequate operation
- AC-4: Multiple start calls return ESP_ERR_INVALID_STATE without creating duplicate tasks
- AC-5: display_logic_stop() deletes running task and clears all LEDs
- AC-6: Stop function can be called when task is not running without error
- AC-7: display_logic_is_running() returns current task state accurately
- AC-8: Task warns if distance sensor is not running during start

### REQ-DSP-3: Distance-to-LED Mapping

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-DSP-1  
**Description**: The system SHALL provide linear mapping of distance measurements to LED positions with configurable range parameters.

**Rationale**: Transforms continuous distance values into discrete LED positions for visual representation, using the distance range specified in configuration.

**Acceptance Criteria**:

- AC-1: Linear mapping function maps distance range to LED indices 0-39
- AC-2: Distance values within [min_distance_cm, max_distance_cm] map to LEDs 0-39
- AC-3: Distance below min_distance_cm returns LED index -1 (out of range indicator)
- AC-4: Distance above max_distance_cm returns LED index -1 (out of range indicator)
- AC-5: Mapping calculation uses floating-point arithmetic for precision
- AC-6: LED index bounds-checking ensures result is always 0-39 for valid range
- AC-7: Mapping algorithm: led_index = (distance - min) / (max - min) * 39

### REQ-DSP-4: LED Display Update Logic

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-DSP-3  
**Description**: The system SHALL update LED display based on distance measurement status with appropriate color coding and error indication.

**Rationale**: Provides visual feedback for different measurement conditions, including successful readings, sensor errors, and out-of-range conditions.

**Acceptance Criteria**:

- AC-1: All LEDs cleared before each update to prevent artifacts
- AC-2: DISTANCE_SENSOR_OK status displays green LED at mapped position
- AC-3: Distance below range displays red LED at position 0 (too close indicator)
- AC-4: Distance above range displays red LED at position 39 (too far indicator)
- AC-5: DISTANCE_SENSOR_TIMEOUT status keeps all LEDs off
- AC-6: DISTANCE_SENSOR_OUT_OF_RANGE status displays red LED at position 39
- AC-7: Other sensor errors (NO_ECHO, INVALID_READING) display red LED at position 0
- AC-8: led_show() called after each update to refresh physical LEDs
- AC-9: Debug logging includes distance value, LED position, and status

### REQ-DSP-5: Distance Measurement Processing

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-DSP-2  
**Description**: The system SHALL continuously process distance measurements from the sensor subsystem with blocking behavior for real-time updates.

**Rationale**: Ensures responsive LED updates when new distance measurements are available while avoiding unnecessary processing when no new data exists.

**Acceptance Criteria**:

- AC-1: Main task loop calls distance_sensor_get_latest() with blocking behavior
- AC-2: Task blocks until new measurement is available (no polling delay)
- AC-3: Each received measurement triggers LED display update
- AC-4: Task continues processing until stopped via display_logic_stop()
- AC-5: Error handling for failed measurement retrieval
- AC-6: Debug logging for each processed measurement
- AC-7: Task runs continuously without artificial delays

---

## Configuration Management

### REQ-DSP-6: Configuration Access

**Type**: Implementation  
**Priority**: Low  
**Description**: The system SHALL provide read access to current display configuration parameters.

**Rationale**: Enables other system components to query current display configuration for integration and debugging purposes.

**Acceptance Criteria**:

- AC-1: display_logic_get_config() returns current configuration structure
- AC-2: Returned configuration reflects values set during initialization
- AC-3: Function can be called without affecting system operation
- AC-4: Returns valid configuration data when initialized

---

## Quality Requirements

### REQ-DSP-7: Display Performance

**Type**: Performance  
**Priority**: Mandatory  
**Description**: Display updates SHALL not introduce perceptible latency in the visualization system.

**Acceptance Criteria**:

- AC-1: LED update processing completes within 10ms of measurement availability
- AC-2: Task priority 3 ensures responsive updates without blocking critical operations
- AC-3: Memory usage remains within 4KB stack allocation
- AC-4: No memory leaks during continuous operation

### REQ-DSP-8: Display Reliability

**Type**: Reliability  
**Priority**: Mandatory  
**Description**: Display logic SHALL handle all sensor error conditions gracefully without system failure.

**Acceptance Criteria**:

- AC-1: Invalid distance values handled without task crashes
- AC-2: Sensor timeout conditions display appropriate status
- AC-3: LED controller errors logged and handled gracefully
- AC-4: Task continues operation despite individual measurement failures
- AC-5: System recovers automatically when sensor operation resumes

---

## Hardware Integration

### REQ-DSP-9: LED Controller Integration

**Type**: Interface  
**Priority**: Mandatory  
**Description**: Display logic SHALL integrate with LED controller component using defined API.

**Rationale**: Ensures proper abstraction between display logic and hardware control while maintaining clear component boundaries.

**Acceptance Criteria**:

- AC-1: Uses led_controller.h API exclusively for LED operations
- AC-2: Calls led_is_initialized() to verify LED controller readiness
- AC-3: Uses led_clear_all(), led_set_pixel(), led_show() functions
- AC-4: Respects LED_COLOR_GREEN and LED_COLOR_RED color definitions
- AC-5: Queries led_get_count() to verify expected LED count

### REQ-DSP-10: Distance Sensor Integration

**Type**: Interface  
**Priority**: Mandatory  
**Description**: Display logic SHALL integrate with distance sensor component using blocking measurement retrieval.

**Rationale**: Provides efficient processing of distance measurements without polling overhead while maintaining component separation.

**Acceptance Criteria**:

- AC-1: Uses distance_sensor.h API exclusively for sensor operations
- AC-2: Calls distance_sensor_is_running() to verify sensor status
- AC-3: Uses distance_sensor_get_latest() for measurement retrieval
- AC-4: Processes all distance_measurement_t status values appropriately
- AC-5: Handles ESP_OK return codes and error conditions

---

## Implementation Assignment

**GitHub Copilot Implementation Scope**: This requirement specification is designated for enhancement and optimization using GitHub Copilot.

**Copilot Assignment Areas**:

1. **Configuration Enhancement**: Dynamic configuration updates and validation
   - **TASK-DSP-001**: Runtime configuration modification support
   - **TASK-DSP-002**: Advanced mapping algorithms (logarithmic, custom curves)

2. **Display Effects**: Enhanced visualization capabilities
   - **TASK-DSP-003**: Smoothing and transition effects
   - **TASK-DSP-004**: Multi-color gradient mapping

3. **Performance Optimization**: Efficiency improvements
   - **TASK-DSP-005**: Memory usage optimization
   - **TASK-DSP-006**: Processing latency reduction

**Human Review Points**:

- Real-time performance validation
- LED display accuracy verification
- Integration testing with sensor and LED components
- Error handling completeness review

---

*This document follows OpenFastTrack methodology for requirements engineering and traceability.*
