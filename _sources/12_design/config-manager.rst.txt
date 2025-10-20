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

   **Description:** Runtime configuration structure holds actively used parameters in RAM for fast access.

   **Current Implementation (Template Scope):**

   The template currently uses configuration for **WiFi credentials only**:

   .. code-block:: c

      typedef struct {
          // WiFi Credentials (Required for network connectivity)
          char wifi_ssid[33];           // WiFi network name (IEEE 802.11 max 32 chars)
          char wifi_password[65];       // WiFi password (WPA max 64 chars)
          
          // Metadata
          uint32_t config_version;      // Schema version for compatibility
          uint32_t save_count;          // Number of saves (statistics)
          
      } system_config_t;

   **WiFi Module Configuration:**

   Other WiFi parameters (AP channel, max connections, STA retries, timeouts) are configured in the WiFi module header file (`main/components/web_server/wifi_manager.h`):

   .. code-block:: c

      // In wifi_manager.h - compile-time defaults
      #define WIFI_AP_CHANNEL           6
      #define WIFI_AP_MAX_CONN          4
      #define WIFI_STA_MAX_RETRY        5
      #define WIFI_STA_TIMEOUT_MS       10000

   **Storage Philosophy:**

   - **NVS Storage** (Runtime configuration): Only user-configurable WiFi credentials (SSID, password)
   - **Compile-Time Headers**: All hardware defaults and WiFi parameters defined in component headers
   - **Rationale**: Minimizes NVS usage, keeps configuration truly minimal and focused

   **Application-Specific Parameters:**

   When forking the template for your project, define application parameters in your component headers, not in NVS:

   .. code-block:: c

      // In my_component/include/my_component.h (NOT in system_config_t)
      #define SENSOR_RANGE_MIN_MM        50
      #define SENSOR_RANGE_MAX_MM        400
      #define LED_COUNT                  30
      #define LED_BRIGHTNESS_DEFAULT     200
      #define MEASUREMENT_INTERVAL_MS    100

   If you need runtime-configurable application parameters (rare), extend `system_config_t` **only for those specific parameters**, not for every possible setting.

   **Design Guidelines for Extensions:**

   1. **Keep WiFi settings in NVS** - Required for user configuration
   2. **Define hardware parameters in headers** - Reduces NVS bloat
   3. **Only add to NVS if truly dynamic** - User needs to change it after deployment
   4. **Use #define for constants** - Compile-time optimization
   5. **Increment config_version** - Only if you add runtime-configurable parameters to struct

   **Reset Requirement After Parameter Changes:**

   ⚠️ **Important**: Configuration changes only take effect after system reset.

   - Changes written to runtime config are visible immediately (for API feedback)
   - Changes persisted to NVS via ``config_save_to_nvs()`` trigger automatic reset
   - Reset ensures all components see new configuration on boot
   - Dynamic reconfiguration is not supported (complexity not justified for IoT devices)

   **Design Rationale:** Reset-after-save prevents inconsistent state where some components use old config while others use new. Simpler than hot-reloading configuration.

   **Type Conversion Strategy:**

   - **NVS Storage**: Use fixed-width integers (uint8_t, uint16_t, uint32_t)
   - **Runtime Use**: Convert to application-friendly types as needed
   - **Example**: Store channel as uint8_t, validate as 1-13 in setter

   **Performance Characteristics:**

   - Setter Validation: <1ms (length check, range validation)
   - Full Validation: <5ms (all parameters checked)
   - NVS Write: <50ms, then reset within 2 seconds
   - Memory Overhead: Structure size < 128 bytes (typical)


API Design
----------

.. spec:: Configuration Parameter Identifiers
   :id: SPEC_CFG_PARAM_1
   :links: REQ_CFG_5
   :status: approved
   :tags: api, parameter

   **Description:** Configuration parameters identified by enum for type-safe access.

   **Parameter Enumeration:**

   .. code-block:: c

      typedef enum {
          // WiFi Credentials
          CONFIG_WIFI_SSID,
          CONFIG_WIFI_PASSWORD,
          
          // Extension point for application parameters
          // CONFIG_APP_PARAM_1,
          // CONFIG_APP_PARAM_2,
          
          CONFIG_PARAM_COUNT  // Sentinel for bounds checking
      } config_param_id_t;

   **Design Rationale:** Enum-based parameter identification enables compile-time type checking and prevents parameter ID collisions when extending configuration.


.. spec:: Configuration API Interface
   :id: SPEC_CFG_API_1
   :links: REQ_CFG_5, REQ_CFG_6
   :status: approved
   :tags: api, interface

   **Description:** Minimal, type-safe API for configuration management supporting uint16 and string types only.

   **Core API Functions:**

   .. code-block:: c

      // Lifecycle management
      esp_err_t config_init(void);
      esp_err_t config_load_from_nvs(system_config_t* config);
      esp_err_t config_save_to_nvs(const system_config_t* config);
      esp_err_t config_factory_reset(void);

      // Type-safe parameter setters (validation at write-time)
      esp_err_t config_set_uint16(config_param_id_t param, uint16_t value);
      esp_err_t config_set_string(config_param_id_t param, const char* value);

      // Type-safe parameter getters (read from runtime config)
      esp_err_t config_get_uint16(config_param_id_t param, uint16_t* value);
      esp_err_t config_get_string(config_param_id_t param, char* value, size_t max_len);

      // Bulk validation (before NVS persistence)
      esp_err_t config_validate(const system_config_t* config, 
                                char* error_msg, size_t msg_len);

   **Typical Usage Pattern:**

   .. code-block:: c

      // Application updates parameters with immediate validation
      if (config_set_string(CONFIG_WIFI_SSID, user_input) != ESP_OK) {
          ESP_LOGE(TAG, "SSID invalid");
          return;
      }
      
      if (config_set_string(CONFIG_WIFI_PASSWORD, password_input) != ESP_OK) {
          ESP_LOGE(TAG, "Password invalid");
          return;
      }
      
      // Validate complete configuration before saving
      char error_msg[128];
      if (config_validate(&runtime_config, error_msg, sizeof(error_msg)) != ESP_OK) {
          ESP_LOGE(TAG, "Configuration invalid: %s", error_msg);
          return;
      }
      
      // Save to NVS (system reset follows)
      config_save_to_nvs(&runtime_config);

   **Thread Safety:** All functions protected by FreeRTOS mutex with configurable timeout.

   **Important Note:** Parameter changes do not take effect until system restarts. Dynamic parameter application is not guaranteed. This is by design: validation at setter time prevents invalid states from propagating into NVS.

   **Design Rationale:**

   - **Type Safety**: Enum + typed setters prevent type confusion and buffer overflows
   - **Validation at Gate**: String parameters length-checked immediately before storage
   - **Simple & Predictable**: Two type-safe functions instead of per-parameter getter/setter explosion
   - **Pragmatic for Embedded C**: Minimal abstraction, direct structure access with safety guardrails
   - **Easy to Extend**: Add new parameters to enum and update validation rules only


.. spec:: Parameter Validation Rules
   :id: SPEC_CFG_VALIDATION_1
   :links: REQ_CFG_6
   :status: approved
   :tags: validation, api

   **Description:** Parameters validated at two points: immediate setter validation and pre-save bulk validation.

   **Validation Strategy:**

   1. **Setter-Time Validation** (Immediate):

      .. code-block:: c

         config_set_string(CONFIG_WIFI_SSID, value)
         // Validates: length 1-32 characters

         config_set_string(CONFIG_WIFI_PASSWORD, value)
         // Validates: length 0-63 characters (0 = open network)

      Returns ESP_ERR_INVALID_ARG on validation failure, preventing invalid values in runtime config.

   2. **Pre-Save Bulk Validation** (Before NVS):

      .. code-block:: c

         config_validate(&config, error_msg, sizeof(error_msg))
         // Validates entire config for semantic consistency
         // Example: check WiFi AP max connections is within device capabilities

   **WiFi Parameter Constraints:**

   - ``CONFIG_WIFI_SSID``: 1-32 characters (IEEE 802.11 max)
   - ``CONFIG_WIFI_PASSWORD``: 0-63 characters (WPA2 max, 0 = open network)

   **Design Rationale:**

   - **Early Rejection**: Invalid strings rejected at setter, preventing buffer corruption
   - **Simple Rules**: Type constraints only (length for strings)
   - **WiFi Defaults**: Channel, max connections, retry, and timeout configured in `wifi_manager.h`
   - **Extension Pattern**: When adding application parameters, define min/max in parameter table

   **Error Handling:** 

   - Setter validation: Return ``ESP_ERR_INVALID_ARG`` immediately
   - Pre-save validation: Return error message describing what's invalid
   - All errors logged with ``ESP_LOGE`` for debugging


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

      GET  /api/config              -> Current configuration (JSON)
      POST /api/config/param        -> Set single parameter
      POST /api/config/validate     -> Validate current config before save
      POST /api/config/apply        -> Apply & save to NVS (triggers reset)
      POST /api/config/reset        -> Factory reset

   **Set Parameter Request:**

   .. code-block:: json

      POST /api/config/param
      Content-Type: application/json
      
      {
        "param": "CONFIG_WIFI_SSID",
        "type": "string",
        "value": "my-network"
      }

   **Response on Error:**

   .. code-block:: json

      {
        "status": "error",
        "code": "INVALID_ARG",
        "message": "SSID too long (max 32 characters)"
      }

   **Get Current Configuration:**

   .. code-block:: json

      GET /api/config
      
      Response:
      {
        "version": "1.0",
        "timestamp": "2025-07-24T12:34:56Z",
        "config": {
          "wifi_ssid": "my-network",
          "wifi_password": "***"
        }
      }

   **Apply & Save:**

   .. code-block:: json

      POST /api/config/apply
      
      Response (before reset):
      {
        "status": "ok",
        "message": "Configuration saved, system resetting..."
      }
      
      Then: System reset within 2 seconds

   **Design Rationale:**

   - **Generic Parameter Endpoint**: Single /api/config/param handles all parameter types
   - **Type Information**: Client specifies type ("string", "uint16") for clarity
   - **Pre-Save Validation**: /api/config/validate allows testing without reset
   - **Simple & Extensible**: Adding new parameters requires only enum update, no new endpoints

   **Authentication:** Currently open (for development). Production deployments should add basic auth or API tokens.


.. spec:: Real-time Configuration Updates
   :id: SPEC_CFG_PREVIEW_1
   :links: REQ_CFG_8
   :status: approved
   :tags: web, ui, lifecycle

   **Description:** Configuration changes validated immediately but only persisted after system reset.

   **Parameter Update Lifecycle:**

   .. code-block:: text

      1. User updates parameter via web UI
         ↓
      2. POST /api/config/param with {param, type, value}
         ↓
      3. config_set_string/uint16() → Setter validates THIS parameter
         ↓
      4. If invalid → error response with details → STOP
      5. If valid → stored in runtime config (RAM)
         ↓
      6. config_validate() → validate entire config for consistency
         ↓
      7. If invalid → error response with details → rollback runtime state → STOP
      8. If valid → response OK "Changes staged, ready to apply"
         ↓
      9. User can update more parameters (go to step 1) or click "Apply & Save"
         ↓
     10. POST /api/config/apply
         ↓
     11. config_save_to_nvs() persists to NVS
         ↓
     12. System reset (to apply changes consistently)

   **Key Design Decisions:**

   1. **Immediate Post-Save Validation**: Every parameter change validated against entire config immediately (step 6-8). User gets feedback about which parameter caused issues, preventing confusion about what went wrong.

   2. **Staged Changes Before Apply**: Parameters can be updated multiple times (steps 1-9). Each update validates individually and against full config. User sees all issues immediately.

   3. **Separate Apply Step**: Actual NVS persistence happens only on explicit "Apply & Save" (steps 10-12). Allows user to review all staged changes before committing.

   4. **Reset After Save**: Ensures all components see new configuration on boot and prevents inconsistent state.

   **Design Rationale:** 
   
   - **Validation Transparency**: User always knows if their changes are valid and why they failed
   - **Multiple Parameter Updates**: Common workflow (change WiFi SSID + password together)
   - **Safety**: Reset-after-save is pragmatic for embedded systems vs. complex hot-reloading
   - **Simple Implementation**: Two-phase approach (stage → apply) without dynamic reconfiguration


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

      [CONFIG] Parameter changed: wifi password -> ****** (user: web)
      [CONFIG] Configuration saved to NVS (save_count: 42)
      [CONFIG] Factory reset performed (reason: user request)

   **Log Levels:**

   - ESP_LOGI: Normal configuration changes
   - ESP_LOGW: Validation failures, preview timeouts
   - ESP_LOGE: NVS errors, corruption detected
   - ESP_LOGD: Detailed debug info (disabled in production)

   **Performance:** Logging does not block configuration operations.


Traceability
------------

All traceability is automatically generated by Sphinx-Needs based on the `:links:` attributes in each specification.

.. needtable::
   :filter: is_ongoing and type == "spec" and "cfg" in id.lower()

.. needflow:: spec
   :filter: is_ongoing and type == "spec" and "cfg" in id.lower()
