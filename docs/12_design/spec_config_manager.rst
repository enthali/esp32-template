Configuration Manager Design Specification
===========================================

This document specifies the design for the Configuration Management System, covering architecture, data structures, API design, and implementation approach.

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

.. spec:: Metadata-Driven Parameter System
   :id: SPEC_CFG_METADATA_1
   :links: REQ_CFG_3, REQ_CFG_4, REQ_CFG_5
   :status: approved
   :tags: data-structure, architecture, metadata

   **Description:** Configuration system uses metadata tables to define parameters generically without hardcoded parameter logic in the config manager.

   **Core Design Principle:** The config manager **knows nothing about WiFi, LEDs, sensors, or any application domain**. It only knows how to store and validate **uint16** and **string** types using metadata provided by the application.

   **Architecture Overview:**

   .. code-block:: text

      ┌─────────────────────────────────────────────────────────┐
      │  Application Layer (main.c, components)                 │
      │  - Defines parameters: CONFIG_WIFI_SSID, CONFIG_LED_CNT │
      │  - Calls: config_set(), config_get()                    │
      └────────────────────┬────────────────────────────────────┘
                           │
      ┌────────────────────▼────────────────────────────────────┐
      │  config.h (Application-Specific)                        │
      │  - Parameter enums (CONFIG_WIFI_SSID = 0, ...)         │
      │  - Metadata table: type, min, max, default              │
      └────────────────────┬────────────────────────────────────┘
                           │
      ┌────────────────────▼────────────────────────────────────┐
      │  config_manager.c (Generic)                             │
      │  - Reads metadata table                                 │
      │  - Validates based on type + min/max                    │
      │  - Stores in NVS using enum-value-as-key                │
      └─────────────────────────────────────────────────────────┘

   **Key Concept:** Adding a new parameter requires **zero changes to config_manager.c**, only:
   
   1. Add enum entry in config.h
   2. Add one line to metadata table in config.h

   **Design Rationale:** Separation of concerns - application defines WHAT to configure, config manager defines HOW to persist/validate.


.. spec:: Dual-Table Parameter System
   :id: SPEC_CFG_TYPES_1
   :links: REQ_CFG_3
   :status: approved
   :tags: data-structure, types, architecture

   **Description:** Configuration system uses **two separate parameter tables** for uint16 and string types to optimize memory usage.

   **Design Rationale:**

   **Problem with Union Approach:** A union containing both uint16 (2 bytes) and string (65 bytes) would waste 63 bytes for every numeric parameter due to union size alignment.

   **Solution:** Separate enums, separate tables, separate runtime caches.

   **Type System Architecture:**

   .. code-block:: c

      // ===== UINT16 Parameters (in config_manager.h) =====
      
      typedef struct {
          uint16_t min;          // Minimum allowed value
          uint16_t max;          // Maximum allowed value
          uint16_t default_val;  // Factory default
      } config_uint16_param_t;

      // ===== STRING Parameters (in config_manager.h) =====
      
      #define CONFIG_STRING_MAX_LEN 64  // Max string length (excluding null)
      
      typedef struct {
          uint8_t min_len;           // Minimum string length
          uint8_t max_len;           // Maximum string length (max 64)
          const char* default_val;   // Pointer to default string in Flash ROM
      } config_string_param_t;

   **User-Defined Parameter Tables (in config.h):**

   .. code-block:: c

      // ===== UINT16 Parameter Enumeration =====
      typedef enum {
          CONFIG_LED_COUNT,        // 0
          CONFIG_LED_BRIGHTNESS,   // 1
          CONFIG_SENSOR_TIMEOUT,   // 2
          // ... user adds more here ...
          CONFIG_UINT16_COUNT      // Auto-counter
      } config_uint16_id_t;

      // ===== UINT16 Parameter Data Table =====
      static const config_uint16_param_t CONFIG_UINT16_PARAMS[CONFIG_UINT16_COUNT] = {
          [CONFIG_LED_COUNT]      = { .min=1,   .max=100,  .default_val=40 },
          [CONFIG_LED_BRIGHTNESS] = { .min=10,  .max=255,  .default_val=128 },
          [CONFIG_SENSOR_TIMEOUT] = { .min=10,  .max=1000, .default_val=100 },
      };

      // ===== STRING Parameter Enumeration =====
      typedef enum {
          CONFIG_WIFI_SSID,        // 0
          CONFIG_WIFI_PASSWORD,    // 1
          CONFIG_DEVICE_NAME,      // 2
          // ... user adds more here ...
          CONFIG_STRING_COUNT      // Auto-counter
      } config_string_id_t;

      // ===== STRING Parameter Data Table =====
      static const config_string_param_t CONFIG_STRING_PARAMS[CONFIG_STRING_COUNT] = {
          [CONFIG_WIFI_SSID]     = { .min_len=1, .max_len=32, .default_val="ESP32-AP" },
          [CONFIG_WIFI_PASSWORD] = { .min_len=0, .max_len=63, .default_val="" },
          [CONFIG_DEVICE_NAME]   = { .min_len=1, .max_len=32, .default_val="ESP32-Device" },
      };

   **Memory Efficiency:**

   .. code-block:: text

      Single Union Approach:
      ────────────────────────────────────────────────────
      union { uint16_t u16; char str[65]; }  // 65 bytes each!
      20 parameters × 65 bytes = 1300 bytes RAM
      
      Dual-Table Approach:
      ────────────────────────────────────────────────────
      uint16_t cache_uint16[15];     // 15 × 2   =  30 bytes
      char cache_strings[5][65];     // 5 × 65   = 325 bytes
                                     ──────────────────────
      Total:                                      355 bytes
      
      **Savings: 73% less RAM!**

   **Type Selection Rationale:**

   - **uint16**: Covers most numeric use cases (LED count, intervals, channel numbers, brightness, timeouts)
   - **string**: Required for WiFi SSID/password, device names, URLs
   - **No floats**: Embedded systems use fixed-point (e.g., temperature * 10 stored as uint16)
   - **No bool**: Use uint16 with 0/1 validation (min=0, max=1)
   - **No int32**: uint16 sufficient for IoT parameters (0-65535 range), saves NVS space

   **Template Example Parameters:**

   The ESP32 template includes example parameters for demonstration purposes:

   - **CONFIG_LED_COUNT**: LED strip length (not used in minimal template, but demonstrates numeric parameter pattern)
   - **CONFIG_LED_BRIGHTNESS**: LED brightness level (example for validation ranges)
   - **CONFIG_WIFI_SSID**: WiFi network name (actual functionality)
   - **CONFIG_WIFI_PASSWORD**: WiFi password (actual functionality)

   Users should keep LED examples in their fork as reference patterns for adding application-specific parameters.

   **Extensibility:** 
   
   Adding new parameter types (uint8, int16, uint32) requires:
   
   1. Define new struct type (e.g., config_uint8_param_t)
   2. Add new enum (e.g., config_uint8_id_t)
   3. Add API functions (e.g., config_get_uint8(), config_set_uint8())
   4. Add runtime cache array in config_manager.c
   5. Update NVS key prefix (e.g., "b" for byte/uint8)


.. spec:: Parameter Metadata Structure
   :id: SPEC_CFG_METADATA_TABLE_1
   :links: REQ_CFG_3, REQ_CFG_6
   :status: approved
   :tags: data-structure, metadata, validation

   **Description:** Parameters organized in two separate metadata tables (uint16 and string), defined by application in config.h.

   **File Organization:**

   .. code-block:: text

      config_manager.h (Generic Framework):
      ├── config_uint16_param_t struct definition
      ├── config_string_param_t struct definition
      └── Generic API declarations
      
      config.h (Application-Specific):
      ├── config_uint16_id_t enum
      ├── CONFIG_UINT16_PARAMS[] table
      ├── config_string_id_t enum
      └── CONFIG_STRING_PARAMS[] table
      
      config_manager.c (Generic Implementation):
      ├── #include "config_manager.h"
      ├── #include "config.h"  ← imports user tables
      ├── Runtime caches (sized by user enums)
      └── Generic get/set implementations

   **Design Benefits:**

   - **Separation of Concerns**: Framework (config_manager.h/.c) vs Application (config.h)
   - **Zero Code Changes**: Adding parameters requires only config.h edits
   - **Compile-Time Safety**: Designated initializers catch enum-index mismatches
   - **Flash Storage**: const tables stored in ROM, zero RAM cost
   - **Type Safety**: Separate enums prevent mixing uint16/string parameter IDs

   **Example Application Definition (config.h):**

   .. code-block:: c

      #include "config_manager.h"  // Import struct definitions

      // ===== UINT16 Parameters =====
      typedef enum {
          CONFIG_LED_COUNT,        // Example: LED strip configuration
          CONFIG_LED_BRIGHTNESS,   // Example: LED brightness control
          CONFIG_UINT16_COUNT
      } config_uint16_id_t;

      static const config_uint16_param_t CONFIG_UINT16_PARAMS[CONFIG_UINT16_COUNT] = {
          [CONFIG_LED_COUNT] = {
              .min = 1,
              .max = 100,
              .default_val = 40
          },
          [CONFIG_LED_BRIGHTNESS] = {
              .min = 10,
              .max = 255,
              .default_val = 128
          },
      };

      // ===== STRING Parameters =====
      typedef enum {
          CONFIG_WIFI_SSID,
          CONFIG_WIFI_PASSWORD,
          CONFIG_STRING_COUNT
      } config_string_id_t;

      static const config_string_param_t CONFIG_STRING_PARAMS[CONFIG_STRING_COUNT] = {
          [CONFIG_WIFI_SSID] = {
              .min_len = 1,
              .max_len = 32,  // IEEE 802.11 max SSID length
              .default_val = "ESP32-AP"  // Pointer to const string in Flash
          },
          [CONFIG_WIFI_PASSWORD] = {
              .min_len = 0,   // 0 = open network allowed
              .max_len = 63,  // WPA2 max password length
              .default_val = ""
          },
      };

   **Template Scope:**

   The ESP32 template includes:

   - **Functional Parameters**: CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD (used by WiFi manager)
   - **Example Parameters**: CONFIG_LED_COUNT, CONFIG_LED_BRIGHTNESS (demonstrate patterns, not connected to hardware)

   **Design Rationale for Examples:**

   LED parameters are intentionally included as **reference implementations** even though the minimal template has no LED hardware. They demonstrate:

   - Numeric parameter validation patterns
   - Range constraint examples (min=1, max=100)
   - Typical IoT device parameters

   Users forking the template can:

   1. Keep LED examples as-is (no harm, minimal RAM cost)
   2. Replace with their own parameters (sensors, motors, etc.)
   3. Extend with additional parameters following the same pattern


.. spec:: NVS Storage Format
   :id: SPEC_CFG_STORAGE_1
   :links: REQ_CFG_4
   :status: approved
   :tags: storage, nvs

   **Description:** Configuration parameters stored individually in NVS using type-prefixed enum value as key.

   **Storage Strategy:**

   .. code-block:: c

      // UINT16 parameters use prefix "u"
      CONFIG_LED_COUNT = 0      → NVS key "u0" → nvs_set_u16(handle, "u0", 40)
      CONFIG_LED_BRIGHTNESS = 1 → NVS key "u1" → nvs_set_u16(handle, "u1", 128)
      
      // STRING parameters use prefix "s"
      CONFIG_WIFI_SSID = 0      → NVS key "s0" → nvs_set_str(handle, "s0", "ESP32-AP")
      CONFIG_WIFI_PASSWORD = 1  → NVS key "s1" → nvs_set_str(handle, "s1", "secret")

   **Key Generation:**

   .. code-block:: c

      // For uint16 parameters
      esp_err_t config_set_uint16(config_uint16_id_t id, uint16_t value) {
          // Generate NVS key: "u" + enum value
          char key[4];
          snprintf(key, sizeof(key), "u%d", id);
          
          // Lookup metadata for validation
          const config_uint16_param_t* param = &CONFIG_UINT16_PARAMS[id];
          
          // Validate range
          if (value < param->min || value > param->max) {
              ESP_LOGE(TAG, "Value out of range");
              return ESP_ERR_INVALID_ARG;
          }
          
          // Store in NVS
          return nvs_set_u16(nvs_handle, key, value);
      }

      // For string parameters
      esp_err_t config_set_string(config_string_id_t id, const char* value) {
          char key[4];
          snprintf(key, sizeof(key), "s%d", id);  // "s" prefix for strings
          
          // Validate length
          const config_string_param_t* param = &CONFIG_STRING_PARAMS[id];
          size_t len = strlen(value);
          if (len < param->min_len || len > param->max_len) {
              ESP_LOGE(TAG, "String length invalid");
              return ESP_ERR_INVALID_ARG;
          }
          
          // Store in NVS
          return nvs_set_str(nvs_handle, key, value);
      }

   **Storage Optimization:**

   - **No Metadata in NVS**: Only parameter values stored, metadata stays in Flash ROM
   - **Compact Keys**: 2-3 character keys ("u0", "s1", "u42") minimize NVS overhead
   - **Type-Specific Storage**: Uses correct NVS function (nvs_set_u16 vs nvs_set_str)
   - **Independent Parameters**: Each parameter stored separately for partial updates
   - **Type Prefix**: "u" and "s" prefixes prevent key collisions between parameter types

   **NVS Namespace:** All configuration parameters stored in namespace "config" for isolation from other NVS users.

   **Migration Strategy:** 
   
   Enum value IS part of the storage key, so:
   
   - ✅ **Reordering enums breaks compatibility** - Users must bump config version and handle migration
   - ✅ **Appending is safe** - Add new parameters at end (before _COUNT sentinel)
   - ✅ **Separate uint16/string enums** - Can reorder independently (different key prefixes)
   
   **Key Capacity:** Max 999 parameters per type (keys "u0" to "u999", "s0" to "s999")


.. spec:: Runtime Configuration Structure
   :id: SPEC_CFG_RUNTIME_1
   :links: REQ_CFG_3
   :status: deprecated
   :tags: data-structure, runtime

   **Description:** **DEPRECATED** - This specification is replaced by SPEC_CFG_METADATA_1 (metadata-driven parameter system).

   **Old Approach:** Monolithic system_config_t structure with all parameters hardcoded.

   **New Approach:** Runtime cache implemented as array of config_value_t, indexed by enum:

   .. code-block:: c

      // Runtime cache (in config_manager.c, private)
      static config_value_t runtime_cache[CONFIG_PARAM_COUNT];
      
      // Access via config_get()/config_set() API
      config_get(CONFIG_WIFI_SSID, &value);  // → runtime_cache[2]
      config_get(CONFIG_LED_COUNT, &value);  // → runtime_cache[0]

   **Migration Notes:** Application code should use config_get/set() instead of direct structure access.


API Design
----------

.. spec:: Type-Specific Configuration API
   :id: SPEC_CFG_API_1
   :links: REQ_CFG_5
   :status: approved
   :tags: api, interface

   **Description:** Type-specific API with separate functions for uint16 and string parameters.

   **Core API Functions:**

   .. code-block:: c

      // ====== Lifecycle Management ======
      
      /**
       * @brief Initialize configuration manager
       * 
       * Opens NVS namespace "config", loads all uint16 and string parameters
       * from NVS into runtime caches. If NVS is empty or corrupted, loads
       * default values from parameter tables and persists them.
       * 
       * @return ESP_OK on success
       * @return ESP_ERR_NVS_* on NVS initialization failure
       * @return ESP_ERR_NO_MEM if memory allocation fails
       */
      esp_err_t config_init(void);

      /**
       * @brief Reset all parameters to factory defaults
       * 
       * Loads default values from CONFIG_UINT16_PARAMS and CONFIG_STRING_PARAMS
       * tables, erases all NVS entries in "config" namespace, and persists
       * defaults to NVS.
       * 
       * @return ESP_OK on success
       * @return ESP_ERR_NVS_* on NVS operation failure
       */
      esp_err_t config_factory_reset(void);

      // ====== UINT16 Parameter Access ======
      
      /**
       * @brief Get uint16 parameter value
       * 
       * Reads parameter from runtime cache (RAM) without NVS access.
       * Fast operation (<10 CPU cycles).
       * 
       * @param id Parameter identifier from config_uint16_id_t enum
       * @param value Pointer to store parameter value
       * @return ESP_OK on success
       * @return ESP_ERR_INVALID_ARG if id out of range or value is NULL
       */
      esp_err_t config_get_uint16(config_uint16_id_t id, uint16_t* value);

      /**
       * @brief Set uint16 parameter value
       * 
       * Validates parameter using metadata table (min, max constraints),
       * updates runtime cache, and persists to NVS.
       * 
       * @param id Parameter identifier from config_uint16_id_t enum
       * @param value New parameter value
       * @return ESP_OK on success
       * @return ESP_ERR_INVALID_ARG if value out of range
       * @return ESP_ERR_NVS_* on NVS write failure
       */
      esp_err_t config_set_uint16(config_uint16_id_t id, uint16_t value);

      // ====== STRING Parameter Access ======
      
      /**
       * @brief Get string parameter value
       * 
       * Copies string from runtime cache to user buffer.
       * 
       * @param id Parameter identifier from config_string_id_t enum
       * @param buffer Buffer to store string (must be at least buf_len bytes)
       * @param buf_len Maximum buffer size (including null terminator)
       * @return ESP_OK on success
       * @return ESP_ERR_INVALID_ARG if id out of range or buffer is NULL
       * @return ESP_ERR_INVALID_SIZE if buffer too small
       */
      esp_err_t config_get_string(config_string_id_t id, char* buffer, size_t buf_len);

      /**
       * @brief Set string parameter value
       * 
       * Validates string length using metadata table (min_len, max_len),
       * copies to runtime cache, and persists to NVS.
       * 
       * @param id Parameter identifier from config_string_id_t enum
       * @param value Null-terminated string value
       * @return ESP_OK on success
       * @return ESP_ERR_INVALID_ARG if value is NULL or length out of range
       * @return ESP_ERR_NVS_* on NVS write failure
       */
      esp_err_t config_set_string(config_string_id_t id, const char* value);

   **Typical Usage Pattern:**

   .. code-block:: c

      // ===== Application Code (web_server.c) =====
      
      // Set WiFi SSID from user input
      if (config_set_string(CONFIG_WIFI_SSID, user_ssid) != ESP_OK) {
          ESP_LOGE(TAG, "SSID validation failed");
          return HTTP_400_BAD_REQUEST;
      }
      
      // Set WiFi password
      if (config_set_string(CONFIG_WIFI_PASSWORD, user_password) != ESP_OK) {
          ESP_LOGE(TAG, "Password validation failed");
          return HTTP_400_BAD_REQUEST;
      }
      
      // Set LED count from slider (example parameter)
      if (config_set_uint16(CONFIG_LED_COUNT, led_count_slider) != ESP_OK) {
          ESP_LOGE(TAG, "LED count out of range");
          return HTTP_400_BAD_REQUEST;
      }
      
      // Read current LED brightness
      uint16_t brightness;
      config_get_uint16(CONFIG_LED_BRIGHTNESS, &brightness);
      
      ESP_LOGI(TAG, "Configuration updated successfully");

   **API Design Principles:**

   1. **Type-Specific Functions**: Separate get/set for uint16 and string parameters
   2. **Compile-Time Type Safety**: Separate enums prevent mixing parameter types
   3. **Immediate Validation**: Parameters validated at set-time, no invalid values reach NVS
   4. **No Domain Knowledge**: Config manager never mentions WiFi, LED, or any application concept
   5. **Fast Reads**: Reads from RAM cache, no NVS access
   6. **Atomic Writes**: Each config_set() immediately persists to NVS

   **Thread Safety:** All functions protected by FreeRTOS mutex (configurable timeout).

   **Error Logging:** All validation failures logged with ESP_LOGE, including parameter ID and constraint values.


.. spec:: Metadata-Driven Parameter Validation
   :id: SPEC_CFG_VALIDATION_1
   :links: REQ_CFG_6
   :status: approved
   :tags: validation, api

   **Description:** Parameters validated using metadata table constraints, no hardcoded validation logic.

   **Validation Implementation:**

   .. code-block:: c

      // UINT16 validation
      esp_err_t config_validate_uint16(config_uint16_id_t id, uint16_t value) {
          // Bounds check
          if (id >= CONFIG_UINT16_COUNT) {
              ESP_LOGE(TAG, "Invalid uint16 parameter ID: %d", id);
              return ESP_ERR_INVALID_ARG;
          }
          
          // Lookup metadata
          const config_uint16_param_t* param = &CONFIG_UINT16_PARAMS[id];
          
          // Range validation
          if (value < param->min || value > param->max) {
              ESP_LOGE(TAG, "Parameter %d out of range: %u (min=%u, max=%u)",
                       id, value, param->min, param->max);
              return ESP_ERR_INVALID_ARG;
          }
          
          return ESP_OK;
      }

      // STRING validation
      esp_err_t config_validate_string(config_string_id_t id, const char* value) {
          // Bounds check
          if (id >= CONFIG_STRING_COUNT) {
              ESP_LOGE(TAG, "Invalid string parameter ID: %d", id);
              return ESP_ERR_INVALID_ARG;
          }
          
          // NULL check
          if (value == NULL) {
              ESP_LOGE(TAG, "String parameter %d: NULL value not allowed", id);
              return ESP_ERR_INVALID_ARG;
          }
          
          // Lookup metadata
          const config_string_param_t* param = &CONFIG_STRING_PARAMS[id];
          
          // Length validation
          size_t len = strlen(value);
          if (len < param->min_len || len > param->max_len) {
              ESP_LOGE(TAG, "String parameter %d length invalid: %u (min=%u, max=%u)",
                       id, len, param->min_len, param->max_len);
              return ESP_ERR_INVALID_ARG;
          }
          
          return ESP_OK;
      }

   **Validation Rules:**

   - **uint16 parameters**: Value must be within [min, max] inclusive
   - **string parameters**: strlen(value) must be within [min_len, max_len] inclusive
   - **Empty strings**: Allowed only if min_len == 0 (e.g., WiFi password for open networks)
   - **NULL strings**: Never allowed, validation returns ESP_ERR_INVALID_ARG

   **Validation Timing:**

   - Called automatically by config_set_uint16() / config_set_string() before NVS write
   - Called during config_init() for all cached parameters (integrity check)
   - Called by config_factory_reset() for default values (sanity check)

   **Error Handling:**

   - Invalid parameters rejected immediately with ESP_ERR_INVALID_ARG
   - Detailed error logged with parameter ID, value, and constraints
   - Runtime cache NOT updated if validation fails
   - NVS NOT written if validation fails

   **Example Validation Scenarios:**

   .. code-block:: c

      // LED Count: min=1, max=100
      config_set_uint16(CONFIG_LED_COUNT, 0);    // ❌ FAIL: below min
      config_set_uint16(CONFIG_LED_COUNT, 50);   // ✅ PASS: within range
      config_set_uint16(CONFIG_LED_COUNT, 200);  // ❌ FAIL: above max

      // WiFi SSID: min_len=1, max_len=32
      config_set_string(CONFIG_WIFI_SSID, "");           // ❌ FAIL: too short
      config_set_string(CONFIG_WIFI_SSID, "MyNetwork");  // ✅ PASS: valid length
      config_set_string(CONFIG_WIFI_SSID, "Very_Long_SSID_Name_Exceeding_Limit_XYZ");  // ❌ FAIL: too long

      // WiFi Password: min_len=0, max_len=63
      config_set_string(CONFIG_WIFI_PASSWORD, "");       // ✅ PASS: open network
      config_set_string(CONFIG_WIFI_PASSWORD, "secret"); // ✅ PASS: normal password

   **Design Rationale:** 
   
   Metadata-driven validation requires zero code changes when adding new parameters. Application developer only edits config.h metadata tables. Config manager implementation (config_manager.c) never needs modification for new parameter types.


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

   .. code-block:: http

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

   .. code-block:: http

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

   .. code-block:: http

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
   :columns: id, title, status, tags

.. needflow:: SPEC_CFG_ARCH_1
