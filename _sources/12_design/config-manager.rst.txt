Configuration Manager Design Specification
===========================================

This document specifies the design for the Configuration Management System, covering architecture, data structures, API design, and implementation approach.

Document Metadata
-----------------

:Version: 1.0
:Date: 2025-07-24
:Author: ESP32 Template Team
:Requirements: REQ_CFG_1 through REQ_CFG_11


Architecture Design
-------------------

.. spec:: Layered Configuration Architecture
   :id: SPEC_CFG_ARCH_1
   :links: REQ_CFG_1, REQ_CFG_2
   :status: approved
   :tags: architecture, config

   **Description:** The configuration system implements a three-layer architecture for separation of concerns.

   **Architecture Layers:**

   .. code-block:: text

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

   **Layer Responsibilities:**

   - **Application Layer**: Consumes configuration values through standardized API
   - **Configuration API Layer**: Provides thread-safe access, validation, and caching
   - **Storage Layer**: Handles NVS operations, persistence, and error recovery

   **Design Rationale:** Layered architecture enables independent testing, simplifies maintenance, and provides clear separation between business logic and storage concerns.


.. spec:: Configuration Data Flow
   :id: SPEC_CFG_FLOW_1
   :links: REQ_CFG_3, REQ_CFG_4, REQ_CFG_5
   :status: approved
   :tags: architecture, dataflow

   **Description:** Configuration data flows through the system with caching and validation.

   **Data Flow Stages:**

   .. code-block:: text

      Startup:  NVS → config_load() → Runtime Cache → Application
      Runtime:  Application → config_get() → Runtime Cache
      Update:   Web Interface → config_save() → Validation → NVS → Runtime Cache
      Factory:  config_factory_reset() → Defaults → NVS → Runtime Cache

   **Key Design Decisions:**

   1. **Runtime Cache**: Configuration cached in RAM for fast access (<10 CPU cycles)
   2. **Atomic Updates**: Configuration changes applied atomically to prevent partial updates
   3. **Validation Gate**: All changes validated before persistence to prevent invalid state
   4. **Error Recovery**: Automatic fallback to defaults on NVS corruption


Data Structure Design
---------------------

.. spec:: NVS Storage Format
   :id: SPEC_CFG_STORAGE_1
   :links: REQ_CFG_3, REQ_CFG_4
   :status: approved
   :tags: data-structure, storage

   **Description:** Configuration stored in NVS as packed 64-byte structure with metadata and checksums.

   **Storage Layout:**

   .. code-block:: c

      typedef struct {
          // Metadata (16 bytes)
          uint32_t magic_number;        // 0xCFG12345 - corruption detection
          uint32_t config_version;      // Schema version for migration
          uint32_t crc32_checksum;      // Data integrity verification  
          uint32_t save_count;          // Change tracking
          
          // Component configurations (~48 bytes)
          // ... component-specific fields ...
          
          // Reserved for expansion
          uint8_t reserved[N];          // Future-proofing
          
      } __attribute__((packed)) config_nvs_storage_t;

   **Storage Optimization:**

   - Fixed Size: 64-byte structure for efficient NVS operations
   - Alignment: Packed structure to minimize flash usage
   - Reserved Fields: Expansion capability without breaking compatibility
   - Checksum: CRC32 for corruption detection and recovery

   **Migration Strategy:** Version field enables schema migration when structure changes.


.. spec:: Runtime Configuration Structure
   :id: SPEC_CFG_RUNTIME_1
   :links: REQ_CFG_3
   :status: approved
   :tags: data-structure, runtime

   **Description:** Runtime configuration structure mirrors NVS layout but uses application-friendly types.

   **Structure Definition:**

   .. code-block:: c

      typedef struct {
          // Application uses native types for convenience
          float distance_min_cm;        // Converted from mm storage
          float distance_max_cm;        // Converted from mm storage
          uint16_t measurement_interval_ms;
          uint32_t sensor_timeout_ms;
          float smoothing_alpha;        // Converted from fixed-point
          
          uint8_t led_count;
          uint8_t led_brightness;
          
          // ... other component configurations ...
          
      } system_config_t;

   **Type Conversion:** NVS uses integers/fixed-point for efficiency; runtime uses floats for convenience.


API Design
----------

.. spec:: Configuration API Interface
   :id: SPEC_CFG_API_1
   :links: REQ_CFG_5, REQ_CFG_6
   :status: approved
   :tags: api, interface

   **Description:** Public API provides thread-safe access to configuration with validation.

   **Core API Functions:**

   .. code-block:: c

      // Lifecycle management
      esp_err_t config_init(void);
      esp_err_t config_load_from_nvs(void);
      esp_err_t config_save_to_nvs(void);
      esp_err_t config_factory_reset(void);

      // Individual getters (thread-safe, no blocking)
      uint16_t config_get_measurement_interval_ms(void);
      uint8_t config_get_led_count(void);
      uint8_t config_get_led_brightness(void);

      // Individual setters (with validation)
      esp_err_t config_set_measurement_interval(uint16_t interval_ms);
      esp_err_t config_set_led_count(uint8_t count);
      esp_err_t config_set_led_brightness(uint8_t brightness);

      // Bulk operations
      esp_err_t config_get_all(system_config_t* config);
      esp_err_t config_set_all(const system_config_t* config);
      bool config_validate_all(const system_config_t* config, 
                               char* error_msg, size_t msg_len);

   **Thread Safety:** All functions protected by FreeRTOS mutex with configurable timeout.


.. spec:: Parameter Validation Rules
   :id: SPEC_CFG_VALIDATION_1
   :links: REQ_CFG_6
   :status: approved
   :tags: validation, api

   **Description:** All configuration parameters validated against defined ranges before acceptance.

   **Validation Rules:**

   - distance_min: 5.0 to 100.0 cm
   - distance_max: 20.0 to 400.0 cm, must be > distance_min
   - measurement_interval: 50 to 1000 ms
   - sensor_timeout: 100 to 5000 ms
   - led_count: 1 to 60
   - led_brightness: 10 to 255

   **Error Handling:** Invalid values return ESP_ERR_INVALID_ARG with descriptive error message.

   **Validation Timing:** Performed before NVS write to prevent persisting invalid state.


Web Interface Design
--------------------

.. spec:: Configuration REST API
   :id: SPEC_CFG_WEB_API_1
   :links: REQ_CFG_7, REQ_CFG_8, REQ_CFG_9
   :status: approved
   :tags: web, api, rest

   **Description:** RESTful HTTP API for configuration management via web interface.

   **API Endpoints:**

   .. code-block:: text

      GET  /api/config           -> Current configuration (JSON)
      POST /api/config           -> Update configuration (JSON)
      POST /api/config/preview   -> Temporary configuration preview
      POST /api/config/apply     -> Apply previewed configuration
      POST /api/config/reset     -> Factory reset
      GET  /api/config/export    -> Export configuration (JSON download)
      POST /api/config/import    -> Import configuration (JSON upload)

   **JSON Format:**

   .. code-block:: json

      {
        "version": "1.0",
        "timestamp": "2025-07-24T12:34:56Z",
        "config": {
          "measurement_interval_ms": 100,
          "led_count": 30,
          "led_brightness": 128
        }
      }

   **Authentication:** Currently open; future versions may add basic auth or API tokens.


.. spec:: Real-time Configuration Preview
   :id: SPEC_CFG_PREVIEW_1
   :links: REQ_CFG_8
   :status: approved
   :tags: web, ui, preview

   **Description:** Web interface supports temporary configuration preview with auto-revert.

   **Preview Workflow:**

   1. User adjusts sliders on /settings page
   2. JavaScript POSTs to /api/config/preview
   3. ESP32 applies changes temporarily (30-second timeout)
   4. User sees real-time visual feedback (e.g., LED brightness)
   5. User clicks "Apply" to persist, or timeout auto-reverts

   **Safety Feature:** Preview timeout prevents bricking device if user navigates away.

   **Implementation:** Separate preview_config buffer, swapped with active config on apply.


.. spec:: Web Settings Page
   :id: SPEC_CFG_UI_1
   :links: REQ_CFG_7, REQ_WEB_2
   :status: approved
   :tags: web, ui, html

   **Description:** HTML5 settings page with form validation and real-time updates.

   **Page Structure:**

   .. code-block:: text

      <form id="config-form">
        <fieldset>
          <legend>Component Settings</legend>
          <input type="range" id="param1" min="X" max="Y" step="Z">
          <span class="value-display">Current: <span id="param1-value"></span></span>
        </fieldset>
        
        <div class="config-actions">
          <button type="button" id="preview-btn">Preview</button>
          <button type="button" id="apply-btn">Apply &amp; Save</button>
          <button type="button" id="reset-btn">Factory Reset</button>
        </div>
      </form>

   **JavaScript Behavior:**

   - Range inputs trigger live preview (debounced 500ms)
   - Value displays update in real-time
   - Factory reset requires confirmation dialog
   - Success/error messages displayed via toast notifications


Error Handling Design
---------------------

.. spec:: NVS Error Recovery Strategy
   :id: SPEC_CFG_ERROR_1
   :links: REQ_CFG_10
   :status: approved
   :tags: error-handling, nvs

   **Description:** Configuration system handles NVS failures gracefully with automatic recovery.

   **Error Recovery Strategy:**

   1. **Corruption Detection**: CRC32 mismatch triggers recovery
   2. **Magic Number Check**: Invalid magic triggers factory reset
   3. **Version Migration**: Unsupported version triggers controlled migration or reset
   4. **Fallback to Defaults**: Any read failure results in default configuration
   5. **Logging**: All errors logged with ESP_LOGE for debugging

   **Boot Behavior:** System always boots successfully even with corrupted NVS.

   **User Notification:** Web interface displays warning banner if defaults used.


.. spec:: Configuration Logging
   :id: SPEC_CFG_LOGGING_1
   :links: REQ_CFG_11
   :status: approved
   :tags: logging, debugging

   **Description:** All configuration changes logged for debugging and audit trail.

   **Logging Format:**

   .. code-block:: text

      [CONFIG] Parameter changed: led_brightness 128 -> 255 (user: web)
      [CONFIG] Configuration saved to NVS (save_count: 42)
      [CONFIG] Factory reset performed (reason: user request)

   **Log Levels:**

   - ESP_LOGI: Normal configuration changes
   - ESP_LOGW: Validation failures, preview timeouts
   - ESP_LOGE: NVS errors, corruption detected
   - ESP_LOGD: Detailed debug info (disabled in production)

   **Performance:** Logging does not block configuration operations.


Implementation Strategy
-----------------------

.. spec:: Phased Implementation Approach
   :id: SPEC_CFG_IMPL_1
   :links: REQ_CFG_1, REQ_CFG_2, REQ_CFG_3, REQ_CFG_4, REQ_CFG_5
   :status: approved
   :tags: implementation, strategy

   **Description:** Configuration system implemented in phases to manage complexity.

   **Phase 1: Core Configuration (Milestone 1)**

   - Implement config.h centralized header (REQ_CFG_1, REQ_CFG_2)
   - Define system_config_t structure (REQ_CFG_3)
   - Implement NVS storage layout (REQ_CFG_4)
   - Create basic getter/setter API (REQ_CFG_5)

   **Phase 2: Web Interface (Milestone 2)**

   - Implement REST API endpoints (REQ_CFG_7)
   - Create /settings HTML page (REQ_CFG_8)
   - Add preview functionality (REQ_CFG_9)

   **Phase 3: Advanced Features (Milestone 3)**

   - Add parameter validation (REQ_CFG_6)
   - Implement error recovery (REQ_CFG_10)
   - Add comprehensive logging (REQ_CFG_11)

   **Testing Strategy:** Each phase includes unit tests and integration tests before proceeding.


Traceability
------------

**Requirements Coverage:**

This design specification implements the following requirements:

- :need:`REQ_CFG_1` - Centralized configuration
- :need:`REQ_CFG_2` - Compile-time access
- :need:`REQ_CFG_3` - Runtime configuration structure
- :need:`REQ_CFG_4` - Persistent NVS storage
- :need:`REQ_CFG_5` - Configuration read API
- :need:`REQ_CFG_6` - Parameter validation
- :need:`REQ_CFG_7` - Web interface API
- :need:`REQ_CFG_8` - Configuration save operation
- :need:`REQ_CFG_9` - Reset/reload functionality
- :need:`REQ_CFG_10` - NVS error recovery
- :need:`REQ_CFG_11` - Configuration logging

**Design Specifications:**

- SPEC_CFG_ARCH_1: Architecture design
- SPEC_CFG_FLOW_1: Data flow design
- SPEC_CFG_STORAGE_1: NVS storage format
- SPEC_CFG_RUNTIME_1: Runtime data structure
- SPEC_CFG_API_1: Public API interface
- SPEC_CFG_VALIDATION_1: Validation rules
- SPEC_CFG_WEB_API_1: REST API design
- SPEC_CFG_PREVIEW_1: Preview functionality
- SPEC_CFG_UI_1: Web UI design
- SPEC_CFG_ERROR_1: Error recovery
- SPEC_CFG_LOGGING_1: Logging design
- SPEC_CFG_IMPL_1: Implementation strategy
