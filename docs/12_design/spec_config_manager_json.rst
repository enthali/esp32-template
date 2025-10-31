JSON-Based Configuration Manager Design
========================================

This document specifies the design for the new JSON-Schema-Driven Configuration System, covering architecture, data flow, schema design, and implementation approach.

Architecture Design
-------------------

.. spec:: JSON Schema-Driven Architecture
   :id: SPEC_CFG_JSON_ARCH_1
   :links: REQ_CFG_JSON_1, REQ_CFG_JSON_6
   :status: draft
   :tags: architecture, config, json-schema

   **Description:** The configuration system uses a single JSON schema as source of truth with build-time code generation and runtime NVS storage.

   **Architecture Layers:**

   .. code-block:: text

      ┌──────────────────────────────────────────────┐
      │          Application Layer                   │
      │  (main.c, components)                        │
      │  Uses: config_get_int32("led_count")         │
      ├──────────────────────────────────────────────┤
      │     Configuration API Layer (NVS wrapper)    │
      │  (config_manager.c, config_manager.h)        │
      │  Direct key-based access, no JSON parsing    │
      ├──────────────────────────────────────────────┤
      │         NVS Storage Layer                    │
      │  Direct key→value mapping (no complexity)    │
      ├──────────────────────────────────────────────┤
      │         Metadata (Build-Time)                │
      │  config_schema.json (source of truth)        │
      └──────────────────────────────────────────────┘

   **Browser UI Layer (Separate):**

   .. code-block:: text

      ┌──────────────────────────────────────────────┐
      │      Browser Client (JavaScript)             │
      │  Fetches: /config_schema.json                │
      │  Generates: Form UI dynamically              │
      │  Validates: Client-side (min/max/pattern)    │
      └──────────────────────────────────────────────┘

   **Key Design Principle:** C code does NOT parse JSON at runtime. JSON is embedded as static file for browser only.

   **Layer Responsibilities:**

   - **Application Layer**: Consumes configuration via simple API (config_get_xxx/config_set_xxx)
   - **Config Manager**: Thin NVS wrapper, type-safe getters/setters, no validation logic
   - **NVS Layer**: Simple key-value storage, no complex structures
   - **JSON Schema**: Defines structure, defaults, UI labels, validation rules (build/browser only)


.. spec:: JSON Schema as Single Source of Truth
   :id: SPEC_CFG_JSON_SOURCE_1
   :links: REQ_CFG_JSON_1
   :status: approved
   :tags: architecture, schema, design-pattern

   **Description:** Configuration schema defined once in JSON, used for multiple purposes without duplication.

   **Single Source of Truth Model:**

   .. code-block:: text

      config_schema.json (ONLY definition)
            │
            ├──→ C Code (config_factory_generated.c)
            │    Generates: config_write_factory_defaults() function
            │    Purpose: Initialize NVS with defaults at build-time
            │    When: Build-time Python script (no runtime parsing)
            │
            ├──→ Browser (fetched at runtime)
            │    Fetches: GET /config_schema.json
            │    Purpose: Generate form, validate inputs
            │    When: User loads settings page
            │
            └──→ Documentation/Reference
                 Developers read schema to understand config fields

   **Benefits:**

   1. **No Duplication**: Update schema once, form auto-updates, defaults auto-generated
   2. **Type Safety**: C code validates types match schema (optional validator script)
   3. **Self-Documenting**: Schema contains labels, descriptions, validation rules
   4. **Zero Runtime Overhead**: JSON parsing happens at build-time only, not on ESP32

   **Key Difference from Old System:**

   - Old: Edit config.h structs + config.c defaults + settings.html + JSON serialization (4 places)
   - New: Edit config_schema.json only (1 place), code/UI auto-sync


Data Structure Design
---------------------

.. spec:: Configuration Schema Structure
   :id: SPEC_CFG_JSON_SCHEMA_1
   :links: REQ_CFG_JSON_1, REQ_CFG_JSON_2, REQ_CFG_JSON_3
   :status: approved
   :tags: data-structure, schema

   **Description:** JSON schema defines all configuration fields with metadata for type-safety and UI generation.

   **Schema File Location:** `main/components/config_manager/config_schema.json`

   **Schema Structure:**

   .. code-block:: json

      {
        "schema_version": "1.0",
        "config_namespace": "esp32_app",
        "groups": [
          {
            "id": "wifi",
            "label": "📶 WiFi Settings",
            "description": "Network configuration",
            "order": 1
          }
        ],
        "fields": [
          {
            "key": "wifi_ssid",
            "type": "string",
            "label": "WiFi SSID",
            "default": "",
            "required": true,
            "maxLength": 32,
            "pattern": "^[^\\x00]{1,32}$",
            "group": "wifi",
            "order": 1
          }
        ]
      }

   **Schema Elements:**

   - **schema_version**: Version for future compatibility
   - **config_namespace**: NVS namespace name
   - **groups**: UI section grouping (order matters)
   - **fields**: Individual configuration parameters

   **Field Properties:**

   ============================== ======== =========================================================================
   Property                       Type     Purpose
   ============================== ======== =========================================================================
   `key`                          string   NVS key name (≤15 chars), used directly: config_get("key")
   `type`                         enum     One of: "string", "password", "integer", "boolean", "hidden"
   `label`                        string   Human-readable label for UI
   `default`                      mixed    Default value if NVS not initialized
   `required`                     bool     If true, must have a value
   `group`                        string   Associates field with a group (from groups.id)
   `order`                        int      Display order (within group, lower = first)
   `minLength` / `maxLength`      int      (string/password) Length constraints
   `min` / `max`                  int      (integer) Range constraints
   `step`                         int      (integer) UI increment step
   `pattern`                      regex    (string/password) Validation regex (browser-only)
   ============================== ======== =========================================================================

   **Type Mapping to C API:**

   .. code-block:: c

      // Browser forms generated based on type
      "string"   → <input type="text">
      "password" → <input type="password">
      "integer"  → <input type="number">
      "boolean"  → <input type="checkbox">
      "hidden"   → <input type="hidden">

      // C code uses matching getters
      "string" / "password"  → config_get_string(key, buf, len)
      "integer"             → config_get_int32(key, &value) or config_get_int16(key, &value)
      "boolean"             → config_get_bool(key, &value)
      "hidden"              → config_get_string(key, buf, len)  (internal config)

   **Design Rationale:**

   - **Direct Key Usage**: Using key directly as NVS key eliminates separate UUIDs (saves flash)
   - **Flat Attributes**: No nested validation objects (simpler JSON, smaller file)
   - **Type Defines Input**: Single type field eliminates redundant inputType property
   - **Groups for Organization**: Sections on settings page without separate metadata structure
   - **Order Field**: Ensures predictable UI layout (JSON object order not guaranteed)


.. spec:: Build-Time Code Generation
   :id: SPEC_CFG_JSON_CODEGEN_1
   :links: REQ_CFG_JSON_1, REQ_CFG_JSON_4, REQ_CFG_JSON_5
   :status: approved
   :tags: build-process, code-generation

   **Description:** Python script generates C factory reset function from schema at build time.

   **Code Generation Flow:**

   .. code-block:: text

      config_schema.json
            │
            └──→ tools/generate_config_factory.py (Python 3)
                 │
                 └──→ config_factory_generated.c (auto-generated, compiled)
                      
                      void config_write_factory_defaults(void) {
                          config_set_string("wifi_ssid", "");
                          config_set_string("wifi_password", "");
                          config_set_int32("led_count", 60);
                          // ... auto-generated from schema
                      }

   **Generator Script (~50 lines):**

   .. code-block:: python

      #!/usr/bin/env python3
      import json
      import sys

      def generate_factory_reset(schema_file, output_file):
          with open(schema_file) as f:
              schema = json.load(f)
          
          with open(output_file, 'w') as f:
              f.write('// Auto-generated - DO NOT EDIT\n')
              f.write('#include "config_manager.h"\n\n')
              f.write('void config_write_factory_defaults(void) {\n')
              
              for field in schema['fields']:
                  key = field['key']
                  default = field['default']
                  ftype = field['type']
                  
                  if ftype in ('string', 'password'):
                      f.write(f'    config_set_string("{key}", "{default}");\n')
                  elif ftype == 'integer':
                      f.write(f'    config_set_int32("{key}", {default});\n')
                  elif ftype == 'boolean':
                      val = 'true' if default else 'false'
                      f.write(f'    config_set_bool("{key}", {val});\n')
              
              f.write('}\n')

      if __name__ == '__main__':
          generate_factory_reset(sys.argv[1], sys.argv[2])

   **CMake Integration:**

   .. code-block:: cmake

      # Generate factory defaults from schema
      add_custom_command(
          OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config_factory_generated.c
          COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_config_factory.py
                  ${CMAKE_CURRENT_SOURCE_DIR}/config_schema.json
                  ${CMAKE_CURRENT_BINARY_DIR}/config_factory_generated.c
          DEPENDS config_schema.json tools/generate_config_factory.py
          COMMENT "Generating factory config defaults from schema"
      )

      idf_component_register(
          SRCS "config_manager.c"
               "${CMAKE_CURRENT_BINARY_DIR}/config_factory_generated.c"
          INCLUDE_DIRS "include"
          EMBED_FILES "config_schema.json"
      )

   **Key Benefits:**

   1. **No Runtime JSON Parsing**: C code does not parse JSON at boot
   2. **Type-Safe Factory Defaults**: Python script validates schema during build
   3. **Single Maintenance Point**: Update config_schema.json, everything auto-syncs
   4. **Embedded Schema**: config_schema.json embedded in flash for browser


.. spec:: NVS Storage Format
   :id: SPEC_CFG_JSON_STORAGE_1
   :links: REQ_CFG_JSON_6, REQ_CFG_JSON_8
   :status: approved
   :tags: storage, nvs

   **Description:** Configuration parameters stored in NVS using schema keys directly.

   **Storage Strategy:**

   .. code-block:: c

      // NVS namespace: "config"
      // NVS keys: schema "key" fields (≤15 chars required)

      config_schema.json:
      {
        "key": "wifi_ssid",
        "default": "ESP32-AP"
      }

      Stored in NVS as:
      nvs_set_str(handle, "wifi_ssid", "ESP32-AP");
      
      Retrieved with:
      char ssid[33];
      nvs_get_str(handle, "wifi_ssid", ssid, sizeof(ssid));

   **Key Properties:**

   - **Direct Keys**: NVS key = schema "key" field (no transformation)
   - **Key Length**: Must be ≤15 characters (ESP-IDF NVS constraint)
   - **Type-Specific Storage**: Uses correct NVS function (nvs_set_str, nvs_set_i32, etc.)
   - **Namespace Isolation**: All config in "config" namespace, separate from other NVS users
   - **Simple Structure**: No metadata stored, only values

   **Example NVS Content:**

   .. code-block:: text

      Namespace: config
      ├─ "wifi_ssid"      → "ESP32-AP" (string)
      ├─ "wifi_password"  → "" (string)
      ├─ "led_count"      → 60 (integer)
      └─ "ap_channel"     → 1 (integer)

   **Design Rationale:**

   - **No UUID System**: Using meaningful keys directly is simpler and readable
   - **Minimal Overhead**: NVS overhead minimized with short, direct keys
   - **Easy Debugging**: Readable keys vs. UUID system


NVS Access Layer (Config Manager)
----------------------------------

.. spec:: Type-Safe Configuration API
   :id: SPEC_CFG_JSON_API_1
   :links: REQ_CFG_JSON_7, REQ_CFG_JSON_6
   :status: approved
   :tags: api, interface, c-api

   **Description:** Thin NVS wrapper providing type-safe getters and setters for configuration values.

   **Core API:**

   .. code-block:: c

      // ====== Lifecycle ======
      esp_err_t config_init(void);
      esp_err_t config_factory_reset(void);

      // ====== Type-Safe Getters (read from NVS) ======
      esp_err_t config_get_string(const char* key, char* buf, size_t len);
      esp_err_t config_get_int32(const char* key, int32_t* value);
      esp_err_t config_get_int16(const char* key, int16_t* value);
      esp_err_t config_get_bool(const char* key, bool* value);

      // ====== Type-Safe Setters (write to NVS) ======
      esp_err_t config_set_string(const char* key, const char* value);
      esp_err_t config_set_int32(const char* key, int32_t value);
      esp_err_t config_set_int16(const char* key, int16_t value);
      esp_err_t config_set_bool(const char* key, bool value);

      // ====== Generated Function ======
      void config_write_factory_defaults(void);  // Auto-generated from schema

   **Implementation Characteristics:**

   - **Simple NVS Wrappers**: Each function ~5-10 lines (minimal overhead)
   - **No JSON Parsing**: Direct NVS access, no runtime deserialization
   - **No Validation Logic**: Server trusts client (browser does validation)
   - **No Domain Knowledge**: Functions never mention WiFi, LEDs, etc.
   - **Type Safety by API**: Compiler enforces correct type via function signature

   **Typical Usage:**

   .. code-block:: c

      // Read configuration
      char ssid[33];
      config_get_string("wifi_ssid", ssid, sizeof(ssid));
      
      int32_t led_count;
      config_get_int32("led_count", &led_count);

      // Write configuration
      config_set_string("wifi_ssid", "MyNetwork");
      config_set_int32("led_count", 120);

   **Error Handling:**

   .. code-block:: c

      int32_t value;
      esp_err_t err = config_get_int32("led_count", &value);
      
      if (err != ESP_OK) {
          ESP_LOGW(TAG, "Failed to read led_count: %s", esp_err_to_name(err));
          value = 60;  // Use default
      }

   **Design Rationale:**

   - **No Validation in C**: Validation happens in browser (simpler code)
   - **Direct Key Access**: Eliminates enum systems, more flexible
   - **Key Duplication Accepted**: Key appears in config_schema.json AND C code
     (This is intentional: explicit is better than implicit for embedded)


Web Interface Design
--------------------

.. spec:: JSON Schema for UI Generation
   :id: SPEC_CFG_JSON_UI_1
   :links: REQ_CFG_JSON_10, REQ_CFG_JSON_11
   :status: approved
   :tags: web, ui, javascript

   **Description:** Browser fetches config_schema.json and generates settings form dynamically.

   **Form Generation Flow:**

   .. code-block:: text

      Browser loads /settings.html
            │
            └──→ JavaScript: fetch('/config_schema.json')
                 │
                 └──→ Parse schema, create groups
                      │
                      └──→ For each field:
                           ├─ Create input element (type-specific)
                           ├─ Apply validation attributes (min/max/pattern)
                           ├─ Set label, description
                           └─ Add to corresponding group div

   **Browser-Side Validation:**

   .. code-block:: javascript

      function generateFormFromSchema(schema) {
          for (const group of schema.groups) {
              const groupDiv = createGroupDiv(group);
              
              for (const field of schema.fields) {
                  if (field.group === group.id) {
                      // Create input based on field type
                      const input = createInputElement(field);
                      
                      // Apply validation attributes
                      if (field.type === 'integer') {
                          input.min = field.min;
                          input.max = field.max;
                          input.step = field.step || 1;
                      }
                      
                      if (field.type === 'string' || field.type === 'password') {
                          input.minLength = field.minLength;
                          input.maxLength = field.maxLength;
                          input.pattern = field.pattern;
                      }
                      
                      // Add to form
                      groupDiv.appendChild(createFormGroup(field, input));
                  }
              }
              
              document.getElementById('settings').appendChild(groupDiv);
          }
      }

   **Validation Rules:**

   ============  ====================  ==========================
   Field Type    Browser Validation    Server Trust?
   ============  ====================  ==========================
   string        maxLength, pattern    Yes (no re-validation)
   password      minLength, maxLength  Yes
   integer       min, max, step        Yes
   boolean       HTML5 checkbox        Yes
   ============  ====================  ==========================

   **Design Rationale:**

   - **Browser-Only Validation**: Simple approach for template (no server-side re-validation)
   - **Self-Updating UI**: No need to hardcode form HTML, schema drives generation
   - **Validation Rules as Schema**: Constraints visible in one place


.. spec:: Configuration REST API
   :id: SPEC_CFG_JSON_REST_1
   :links: REQ_CFG_JSON_12, REQ_CFG_JSON_13
   :status: approved
   :tags: web, api, rest

   **Description:** Simple REST API for configuration get/set operations.

   **API Endpoints:**

   .. code-block:: text

      GET  /config_schema.json           -> Embedded JSON schema file
      
      GET  /api/config/all               -> Get all current config values
      GET  /api/config/:key              -> Get single value by key
      
      POST /api/config/:key              -> Set single value
           Body: { "value": "new_value" }
      
      POST /api/config/bulk              -> Set multiple values
           Body: { "key1": "value1", "key2": "value2" }

   **Example: Get Schema**

   .. code-block:: http

      GET /config_schema.json
      
      Response: (200 OK)
      {
        "schema_version": "1.0",
        "groups": [...],
        "fields": [...]
      }

   **Example: Get All Config**

   .. code-block:: http

      GET /api/config/all
      
      Response: (200 OK)
      {
        "wifi_ssid": "MyNetwork",
        "wifi_password": "***",
        "led_count": 60
      }

   **Example: Set Single Value**

   .. code-block:: http

      POST /api/config/wifi_ssid
      Content-Type: application/json
      
      {"value": "NewNetwork"}
      
      Response: (200 OK)
      {"status": "ok", "key": "wifi_ssid", "value": "NewNetwork"}

   **Example: Error Response**

   .. code-block:: http

      POST /api/config/wifi_ssid
      Content-Type: application/json
      
      {"value": ""}  // Empty SSID invalid
      
      Response: (400 Bad Request)
      {
        "status": "error",
        "key": "wifi_ssid",
        "message": "Value cannot be empty"
      }

   **Design Rationale:**

   - **Generic Endpoints**: No need for field-specific endpoints
   - **Key-Based Access**: Matches schema directly (no enum system needed)
   - **Simple Responses**: Status + value (no complex nested structures)


Best Practices & Development Guide
-----------------------------------

.. spec:: Adding New Configuration Fields
   :id: SPEC_CFG_JSON_EXTEND_1
   :links: REQ_CFG_JSON_1, REQ_CFG_JSON_13
   :status: approved
   :tags: development, guide, extensibility

   **Description:** Simple process for adding new configuration fields to schema.

   **Step-by-Step Guide:**

   **1. Define in config_schema.json:**

   .. code-block:: json

      {
        "key": "my_setting",
        "type": "integer",
        "label": "My Custom Setting",
        "description": "This is what my setting does",
        "default": 100,
        "min": 1,
        "max": 1000,
        "step": 10,
        "group": "application",
        "order": 1
      }

   **2. Use in Application Code:**

   .. code-block:: c

      #include "config_manager.h"
      
      int32_t my_setting;
      esp_err_t err = config_get_int32("my_setting", &my_setting);
      
      if (err != ESP_OK) {
          ESP_LOGW(TAG, "Failed to read my_setting: %s", esp_err_to_name(err));
          my_setting = 100;  // Fallback to default
      }
      
      // Use my_setting...

   **3. Web UI Auto-Updates:**

   - Reload settings page
   - New field appears automatically with validation rules
   - Label and description shown from schema

   **Key Naming Rules:**

   - Use `snake_case` (matches C naming conventions)
   - ≤15 characters (NVS key length limit)
   - Avoid special characters except underscore
   - Make names descriptive ("led_count" better than "lc")

   **Benefits of This Approach:**

   - ✅ Single place to define (config_schema.json)
   - ✅ Form auto-generates (no HTML updates needed)
   - ✅ Defaults auto-generated (no C code for factory reset)
   - ✅ Validation auto-applies (schema drives browser validation)


.. spec:: Type Safety Without Code Generation
   :id: SPEC_CFG_JSON_TYPESAFETY_1
   :links: REQ_CFG_JSON_7, REQ_CFG_JSON_14
   :status: approved
   :tags: type-safety, best-practices

   **Description:** Achieving type safety through API design and optional validation rather than mandatory code generation.

   **Type Safety Mechanisms:**

   1. **API Signature Type Safety:**

   .. code-block:: c

      // Compiler enforces types at call site
      int32_t count;
      config_get_int32("led_count", &count);  // ✅ Correct: matches schema type
      
      char* str;
      config_get_int32("wifi_ssid", (int32_t*)str);  // ❌ Logical error (caught by developer testing)

   2. **Schema Documents Correct Type:**

   .. code-block:: json

      {"key": "led_count", "type": "integer"}   → Use config_get_int32()
      {"key": "wifi_ssid", "type": "string"}    → Use config_get_string()

   3. **Optional: Static Validator Script (Nice-to-Have):**

   .. code-block:: python

      # Pre-build validation (not required)
      python3 tools/validate_config_schema.py
      
      # Finds type mismatches in code:
      # ERROR: src/main.c:42 - config_get_int32("wifi_ssid"): 
      #        schema says type="string", not integer

   **When Type Mismatches Happen:**

   - ❌ If schema says `"string"` but code calls `config_get_int32()`:
     - Result: Reads binary garbage as integer
     - Discovery: Runtime error during testing (type mismatch obvious)
     - Recovery: Trivial fix (change one line in C code)

   **For Templates, This is Acceptable:**

   - Small number of config fields (5-10 typically)
   - Type errors obvious after one test run
   - No runtime overhead of validation
   - Code remains simple and understandable

   **When Additional Safety is Needed:**

   - Projects with 20+ config fields
   - Pre-commit validator script (optional)
   - Or: Use code generation approach (separate branch/option)

   **Design Philosophy:**

   - **Explicit > Implicit**: Keys appear in both JSON and C code (obvious when they match)
   - **Simple > Magic**: No hidden code generation unless chosen
   - **Learnable > Complex**: Beginners can understand entire system quickly


Comparison: Old vs New System
-----------------------------

.. spec:: System Comparison
   :id: SPEC_CFG_JSON_COMPARISON_1
   :status: approved
   :tags: documentation, comparison

   **Description:** Side-by-side comparison of old config manager vs. new JSON-based system.

   **Maintenance Effort:**

   ====================  ========================================  ===============================
   Aspect                Old System (config.h)                     New System (JSON Schema)
   ====================  ========================================  ===============================
   Add config field      4 places (struct, defaults, HTML, JSON)   1 place (JSON schema)
   Lines of code         ~30 per field                             ~8 per field
   Default generation    Manual C code                             Auto-generated Python
   Form HTML             Manually written                          Auto-generated JavaScript
   Browser validation    Manual HTML5 attributes                   Auto-applied from schema
   ====================  ========================================  ===============================

   **Code Complexity:**

   ===================  =============================  ================================
   Component            Old System                     New System
   ===================  =============================  ================================
   config_manager.c     200+ lines (complex logic)     ~50 lines (simple NVS wrapper)
   config.h             100+ lines (struct+metadata)   Not needed (JSON replaces)
   settings.html        100+ lines (hardcoded form)    Auto-generated from schema
   Web JavaScript       150+ lines (serialization)     100 lines (generic generation)
   Factory defaults     Manual in code                 Auto-generated from schema
   Total                550+ lines                     200+ lines (50% reduction)
   ===================  =============================  ================================

   **Flash Memory Usage:**

   - Old: config struct + defaults hardcoded in C → ~2KB
   - New: config_schema.json embedded → ~1.5KB
   - Savings: ~500 bytes

   **Runtime Overhead:**

   - Old: Runtime cache + struct manipulation
   - New: Direct NVS access, no JSON parsing
   - Result: **Same or lower overhead**

   **Extensibility:**

   ===================  ==========================  =========================
   Task                 Old System                  New System
   ===================  ==========================  =========================
   Add field            High effort (4 files)       Low effort (1 file)
   Rename field         High risk (4 places)        Low risk (1 place)
   Change defaults      Edit code, rebuild          Edit JSON, rebuild
   Add validation       Code + browser              Browser only
   ===================  ==========================  =========================


Traceability
------------

.. needtable::
   :columns: id, title, status

.. needflow:: SPEC_CFG_JSON_SOURCE_1
