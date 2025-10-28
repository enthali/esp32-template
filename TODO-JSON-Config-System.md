# TODO: JSON-Based Configuration System

**Status:** 💡 Idea / Design Phase  
**Branch:** TBD (create `feature/json-config-system`)  
**Priority:** Medium (Nice-to-have, not critical)  
**Estimated Effort:** 2-3 days full implementation

---

## Problem Statement

Current config system requires triple maintenance:
1. Edit `config.h` structs
2. Update settings HTML forms manually
3. Modify API serialization code

**Pain Points:**
- Config schema spread across C headers, HTML, and JavaScript
- No single source of truth
- Easy to create inconsistencies
- Hard to add new config fields

---

## Proposed Solution: JSON Schema-Driven Config

### Core Concept

**Single JSON file** defines entire configuration:
- Schema (field types, validation)
- Metadata (labels, descriptions, help text)
- Defaults
- UUID-based field identification

### Example: `config_schema.json` (Flash-Optimized)

**Design Principles:**
- ✅ **Use `key` as NVS key directly** - no separate UUID field (saves flash)
- ✅ **No `help` text** - keeps schema small (saves flash)
- ✅ **Flat validation object** - no nested structures (saves flash)
- ✅ **No `placeholder`** - UI can generate from label (saves flash)
- ✅ **`type` defines `inputType`** - string→text, integer→number, password→password
- ✅ **Groups keep `description`** - users need context for sections
- ✅ **Keep `order`** - predictable UI layout matters

**Flash Impact:**
- Removed UUID field: ~15 bytes/field × N fields
- Removed help text: ~50 bytes/field × N fields
- Removed placeholder: ~30 bytes/field × N fields
- Flat validation: ~20 bytes/field × N fields
- **Total savings: ~115 bytes per field!**

```json
{
  "schema_version": "1.0",
  "config_namespace": "esp32_app",
  "groups": [
    {
      "id": "wifi",
      "label": "📶 WiFi Settings",
      "description": "Network configuration",
      "order": 1
    },
    {
      "id": "application",
      "label": "⚙️ Application Settings",
      "description": "General application settings",
      "order": 2
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
    },
    {
      "key": "wifi_password",
      "type": "password",
      "label": "WiFi Password",
      "default": "",
      "minLength": 8,
      "maxLength": 64,
      "group": "wifi",
      "order": 2
    },
    {
      "key": "ap_channel",
      "type": "integer",
      "label": "AP Channel",
      "default": 1,
      "required": true,
      "min": 1,
      "max": 13,
      "group": "wifi",
      "order": 3
    },
    {
      "key": "update_interval_ms",
      "type": "integer",
      "label": "Update Interval",
      "default": 1000,
      "min": 100,
      "max": 10000,
      "step": 100,
      "group": "application",
      "order": 1
    }
  ]
}
```

**UI Generation Rules:**
- `type: "string"` → `<input type="text">`
- `type: "password"` → `<input type="password">`
- `type: "integer"` → `<input type="number">`
- `type: "boolean"` → `<input type="checkbox">`
- `type: "hidden"` → `<input type="hidden">` (for internal config)
- Validation attributes: `min`, `max`, `step`, `minLength`, `maxLength`, `pattern`, `required`

---

## Architecture Options

### Option A: JSON-Only (Runtime Parsing)

**Flow:**
1. ESP32 embeds `config_schema.json` in flash
2. Web UI fetches schema via `/api/config/schema`
3. JavaScript generates settings form dynamically
4. Config values stored in NVS by UUID or key
5. API endpoints use UUID to get/set values

**Pros:**
- ✅ Single source of truth
- ✅ No code generation needed
- ✅ Dynamic, flexible
- ✅ Easy to extend

**Cons:**
- ❌ C code loses type safety (no structs)
- ❌ Runtime JSON parsing overhead
- ❌ More complex NVS access
- ❌ Harder to use config in C code

### Option B: JSON + Code Generation (Build-Time)

**Flow:**
1. `config_schema.json` is source of truth
2. Build script generates:
   - `config_generated.h` (C structs)
   - `config_api_generated.c` (serialization)
   - Settings HTML is auto-generated OR fetches schema
3. C code uses type-safe structs
4. Web UI can still fetch schema for validation

**Pros:**
- ✅ Type-safe C structs
- ✅ Best of both worlds
- ✅ No runtime parsing in C
- ✅ Can still validate in browser

**Cons:**
- ❌ Build-time dependency (Python/Node)
- ❌ Generated code maintenance
- ❌ More complex build system

### Option C: Hybrid (Manual C + JSON Metadata)

**Flow:**
1. Keep C structs in `config.h`
2. Add `config_metadata.json` with UI info only
3. Web UI fetches metadata for form generation
4. C code unchanged (type-safe structs)
5. Manual sync required between C and JSON

**Pros:**
- ✅ Simple, no code generation
- ✅ Type-safe C code
- ✅ Dynamic web UI

**Cons:**
- ❌ Still need to sync C and JSON
- ❌ Partial solution only
- ❌ Easy to get out of sync

---

## Design Decisions (Flash-Optimized)

### 1. Key Strategy (DECIDED: Use key directly)

**Previous approach:** Separate UUID field for reference
**Optimized approach:** Use `key` field as NVS key directly

```c
// Access by key (simple, no UUID needed)
const char* ssid = config_get_string("wifi_ssid");
config_set_string("wifi_ssid", "MyNetwork");
```

**Benefits:**
- ✅ Saves ~15 bytes per field (no UUID field)
- ✅ Simpler C code (direct key access)
- ✅ Less confusing for users
- ✅ Still human-readable

**NVS Limit:** Keys must be ≤15 characters (ESP-IDF constraint)

### 2. Validation Format (DECIDED: Flat attributes)

**Previous approach:** Nested validation object
```json
"validation": {
  "minLength": 8,
  "maxLength": 64,
  "required": true
}
```

**Optimized approach:** Flat field attributes
```json
"minLength": 8,
"maxLength": 64,
"required": true
```

**Benefits:**
- ✅ Saves ~20 bytes per field
- ✅ Easier JavaScript access (`field.required` vs `field.validation.required`)
- ✅ Less nesting = smaller JSON

### 3. Input Type Inference (DECIDED: Type defines input)

**Previous approach:** Separate `inputType` field
```json
"type": "string",
"inputType": "password"
```

**Optimized approach:** Use dedicated type
```json
"type": "password"
```

**Type to Input Mapping:**
- `"string"` → `<input type="text">`
- `"password"` → `<input type="password">`
- `"integer"` → `<input type="number">`
- `"boolean"` → `<input type="checkbox">`
- `"hidden"` → `<input type="hidden">`

**Benefits:**
- ✅ No redundant field (~10 bytes saved)
- ✅ More intuitive schema

### 4. Help Text (DECIDED: Remove to save flash)

**Removed:** `help` field describing each setting

**Rationale:**
- Labels should be self-explanatory
- Can add browser tooltips if needed (not in schema)
- Saves ~50 bytes per field

### 5. Placeholders (DECIDED: Remove to save flash)

**Removed:** `placeholder` field for input hints

**Rationale:**
- UI can auto-generate from label if needed
- Not essential for functionality
- Saves ~30 bytes per field

### 6. Group Descriptions (DECIDED: Keep)

**Kept:** `description` field in groups

**Rationale:**
- Only a few groups (not per-field cost)
- Important for section context
- Users need to understand settings categories
- Minimal flash cost (~50 bytes total)

### 7. Order Field (DECIDED: Keep)

**Kept:** `order` attribute for predictable layout

**Rationale:**
- Critical for UX (consistent UI)
- Only 2 bytes per field
- JSON object order not guaranteed

### 8. NVS Storage Format (DECIDED: Keep current)

**Current:** Keys like `"wifi_ssid"`, `"wifi_password"`

**Decision:** Keep current format for compatibility

**No namespace prefix needed** (keep it simple)

### 9. Settings UI Generation (DECIDED: Runtime JavaScript)

**Approach:** Fetch schema, generate form in JavaScript

```javascript
fetch('/config_schema.json')
  .then(schema => generateFormFromSchema(schema))
```

**Benefits:**
- ✅ No build step
- ✅ Dynamic and flexible
- ✅ Can be updated without reflashing

### 10. Type System (DECIDED: Simple types only)

**Supported Types:**
- `string` - text inputs
- `password` - password inputs
- `integer` - number inputs
- `boolean` - checkboxes
- `hidden` - hidden inputs (for internal config)

**C API Mapping:**
```c
config_get_string(key, buf, len)  // type: "string" or "password"
config_get_int16(key, value)      // type: "integer" with small range
config_get_int32(key, value)      // type: "integer" with large range
config_get_bool(key, value)       // type: "boolean"
```

**No complex types** (enum, float, array) - keep it simple for template

---

## Flash Savings Summary

**Per-Field Savings:**
- UUID field removed: ~15 bytes
- Help text removed: ~50 bytes
- Placeholder removed: ~30 bytes
- Flat validation: ~20 bytes
- No inputType: ~10 bytes
- **Total: ~125 bytes per field**

**For 10 fields: ~1.2 KB saved** 🎉

**Trade-off:** Less verbose schema, still fully functional

---

## Implementation Plan

### Phase 1: Requirements & Design (4 hours)

1. Create Sphinx-Needs requirements document
2. Design complete JSON schema format
3. Define C API for config access
4. Mock up auto-generated settings UI
5. Decide on Option A vs B vs C

**Deliverables:**
- `docs/requirements/req_json_config.rst`
- `docs/design/json-config-design.md`
- Prototype `config_schema.json`

### Phase 2: Proof of Concept (8 hours)

1. Create feature branch: `feature/json-config-system`
2. Implement minimal JSON schema parser
3. Create dynamic form generator in JavaScript
4. Test with 2-3 config fields
5. Validate approach works

**Deliverables:**
- Working prototype with WiFi SSID field
- Performance measurements (RAM, flash)
- Comparison with current approach

### Phase 3: Full Implementation (16 hours)

1. Complete JSON schema with all fields
2. Implement config manager changes
3. Update NVS storage/retrieval
4. Generate/update settings UI
5. Update all API endpoints
6. Migration path for existing configs

**Deliverables:**
- Complete implementation
- Unit tests
- Documentation
- Migration guide

### Phase 4: Testing & Documentation (8 hours)

1. Test all config fields
2. Validate NVS storage
3. Browser compatibility testing
4. Write user documentation
5. Create examples for adding fields

**Deliverables:**
- Test suite
- User guide
- Example config additions

---

## Technical Challenges

### 1. Memory Constraints

**Issue:** JSON parsing requires heap memory

**Mitigations:**
- Parse schema once at startup, cache structure
- Use streaming parser (jsmn, cJSON)
- Consider build-time generation instead

### 2. Type Safety in C

**Issue:** Lose struct-based type checking

**Mitigations:**
- Generate C structs from JSON (Option B)
- Use getter/setter macros with compile-time checks
- Comprehensive unit tests

### 3. NVS Key Length Limit

**Issue:** NVS keys max 15 characters

**Current keys work:** `"wifi_ssid"`, `"ap_channel"`

**Avoid:** Long UUID-based keys would break

### 4. Migration from Current System

**Issue:** Existing devices have current NVS format

**Solution:**
- Keep NVS key format unchanged
- Add version marker in NVS
- Implement migration function

---

## API Design

### New Endpoints

```c
// GET /config_schema.json - Fetch JSON schema
// Response: Full config_schema.json embedded file

// GET /api/config/all - Get all config values
// Response: { "wifi_ssid": "MyNetwork", "ap_channel": 1, ... }

// GET /api/config/:key - Get single value by key
// Response: { "key": "wifi_ssid", "value": "MyNetwork" }

// POST /api/config/bulk - Set multiple values
// Body: { "wifi_ssid": "NewNetwork", "ap_channel": 6 }

// POST /api/config/:key - Set single value
// Body: { "value": "NewNetwork" }
```

### C API

```c
// Type-safe accessors (simple wrappers)
char ssid[33];
config_get_string("wifi_ssid", ssid, sizeof(ssid));

int16_t channel;
config_get_int16("ap_channel", &channel);

config_set_string("wifi_ssid", "NewNetwork");
config_set_int16("ap_channel", 6);

// Factory reset
config_factory_reset();  // Calls generated config_write_factory_defaults()
```

---

## Risks & Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| Increased memory usage | High | Medium | Measure early, optimize parser |
| Lost type safety | Medium | High | Use code generation (Option B) |
| Breaking change | High | Low | Keep NVS format compatible |
| Build complexity | Medium | Medium | Make generation optional |
| Migration issues | High | Low | Test thoroughly, provide fallback |

---

## Success Criteria

### Minimum Viable Product (MVP)

- ✅ JSON schema defines 4+ config fields
- ✅ Settings UI auto-generates from schema
- ✅ Config values persist in NVS
- ✅ No regression in memory/performance
- ✅ Backwards compatible with current NVS

### Full Success

- ✅ All current config fields migrated
- ✅ Documentation for adding new fields
- ✅ <5% memory overhead vs current system
- ✅ Type-safe C API maintained
- ✅ Browser validation matches backend

---

## Alternative: Keep It Simple

**Reality Check:** Is JSON config overkill for a template?

### Arguments AGAINST JSON Config:

1. **Complexity:** Template should be understandable by beginners
2. **Dependencies:** Adding build tools for code generation
3. **Debugging:** Generated code harder to debug
4. **Over-engineering:** Manual forms work fine for templates

### Simpler Alternative: Enhanced Documentation

Instead of JSON config, provide:
1. **Clear example** of adding a config field (step-by-step)
2. **Helper macros** for JSON serialization
3. **Documentation** on config patterns
4. **Template** snippets for common field types

**Effort:** 2-4 hours vs 36+ hours for full JSON system

---

## ⭐ BREAKTHROUGH: Optimized Option A (Evening Session Discovery)

### Key Insight: Eliminate JSON Parsing in C Entirely!

**Problem with Original Option A:** Runtime JSON parsing overhead in C

**Solution:** Build-time code generation + Browser-side validation

### Architecture Flow

```
config_schema.json (source of truth)
    ↓
Python Script (build-time) ────────────────┐
    ↓                                      ↓
config_factory_generated.c          Embedded in Flash
    ↓                                      ↓
Compile to firmware              Browser loads for UI
    ↓
Runtime: NVS only (no JSON!)
```

### Brilliant Realizations

#### 1. Type Safety ✅ SOLVED via API Signatures

```c
// Type-safe by function signature!
int16_t channel;
config_get_int16("c03", &channel);  // Compiler checks type

// Config manager validates JSON type at build time
// Apps get compile-time type safety
```

#### 2. JSON Parsing ✅ ELIMINATED from Runtime

**When JSON is used:**
- ❌ NOT every boot
- ❌ NOT during normal operation  
- ✅ Build-time only (code generation)
- ✅ Factory reset (pre-generated code)
- ✅ Browser (for UI generation)

**In C Runtime:**
```c
// NO JSON parsing needed!
esp_err_t config_get_int16(const char* uuid, int16_t* value) {
    return nvs_get_i16(nvs_handle, uuid, value);  // Direct NVS read
}
```

#### 3. Factory Reset ✅ Code-Generated (No JSON Parser)

**Build-time Python script generates:**

```c
// config_factory_generated.c (auto-generated, never hand-edit)
#include "config_manager.h"

void config_write_factory_defaults(void) {
    config_set_string("wifi_ssid", "");           // Default empty
    config_set_string("wifi_password", "");       // Default empty
    config_set_int16("ap_channel", 1);            // Default channel 1
    config_set_int32("update_interval_ms", 1000); // Default 1 second
    config_set_string("app_name", "ESP32 Template"); // Default name
    // ... auto-generated from config_schema.json
}
```

**Result:** Factory reset without any JSON parsing! 🎉

#### 4. NVS Access ✅ TRIVIAL (key = NVS Key)

```c
// Config manager becomes dead simple
esp_err_t config_set_int16(const char* key, int16_t value) {
    return nvs_set_i16(s_nvs_handle, key, value);
}
```

**Key Design:** Use meaningful keys that fit NVS 15-char limit
```json
{"key": "wifi_ssid"}        // 9 chars ✅
{"key": "wifi_password"}    // 13 chars ✅
{"key": "ap_channel"}       // 10 chars ✅
{"key": "update_int_ms"}    // 13 chars ✅ (shortened if needed)
```

#### 5. Validation ✅ Browser-Only (Zero ESP32 RAM)

```javascript
// Browser loads schema and validates
fetch('/config_schema.json')
  .then(schema => {
    generateFormFromSchema(schema);      // Auto-gen UI
    validateInputs(schema);              // Client-side validation
  });
```

**ESP32 does:** Simple get/set (no validation logic)  
**Browser does:** All validation, form generation, user feedback

### Config Manager Becomes TRIVIAL

**Before:** Complex struct handling, validation, serialization  
**After:** Thin wrapper around NVS

```c
// config_manager.h - Clean API
esp_err_t config_init(void);
esp_err_t config_factory_reset(void);

// Type-safe getters/setters
esp_err_t config_get_string(const char* key, char* buf, size_t len);
esp_err_t config_get_int16(const char* key, int16_t* value);
esp_err_t config_get_int32(const char* key, int32_t* value);
esp_err_t config_get_bool(const char* key, bool* value);

esp_err_t config_set_string(const char* key, const char* value);
esp_err_t config_set_int16(const char* key, int16_t value);
esp_err_t config_set_int32(const char* key, int32_t value);
esp_err_t config_set_bool(const char* key, bool value);

// Generated function (implemented in config_factory_generated.c)
void config_write_factory_defaults(void);
```

**Implementation (~50 lines total!):**

```c
// config_manager.c
static nvs_handle_t s_nvs_handle;

esp_err_t config_init(void) {
    esp_err_t err = nvs_open("config", NVS_READWRITE, &s_nvs_handle);
    if (err != ESP_OK) return err;
    
    // Check if initialized, if not run factory reset
    int32_t init_flag;
    if (nvs_get_i32(s_nvs_handle, "init", &init_flag) != ESP_OK) {
        return config_factory_reset();
    }
    return ESP_OK;
}

esp_err_t config_factory_reset(void) {
    nvs_erase_all(s_nvs_handle);
    config_write_factory_defaults();  // Generated!
    nvs_set_i32(s_nvs_handle, "init", 1);
    return nvs_commit(s_nvs_handle);
}

// Getters/setters are one-liners!
esp_err_t config_get_int16(const char* key, int16_t* value) {
    return nvs_get_i16(s_nvs_handle, key, value);
}

esp_err_t config_set_int16(const char* key, int16_t value) {
    esp_err_t err = nvs_set_i16(s_nvs_handle, key, value);
    if (err == ESP_OK) nvs_commit(s_nvs_handle);
    return err;
}
// ... repeat for string, int32, bool (simple wrappers)
```
    return nvs_get_i16(s_nvs_handle, uuid, value);
}

esp_err_t config_set_int16(const char* uuid, int16_t value) {
    esp_err_t err = nvs_set_i16(s_nvs_handle, uuid, value);
    if (err == ESP_OK) nvs_commit(s_nvs_handle);
    return err;
}
// ... repeat for string, int32, bool (simple wrappers)
```

**That's it!** No complex logic, no validation, no serialization.

### Build Integration

**CMakeLists.txt:**

```cmake
# Generate factory defaults
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config_factory_generated.c
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_config_factory.py
            ${CMAKE_CURRENT_SOURCE_DIR}/config_schema.json
            ${CMAKE_CURRENT_BINARY_DIR}/config_factory_generated.c
    DEPENDS config_schema.json tools/generate_config_factory.py
    COMMENT "Generating factory config defaults from schema"
)

# Embed schema for web UI
set(COMPONENT_EMBED_FILES config_schema.json)

# Include generated source
set(COMPONENT_SRCS 
    "config_manager.c"
    "${CMAKE_CURRENT_BINARY_DIR}/config_factory_generated.c"
)
```

**Python Generator (~50 lines):**

```python
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
            uuid = field['uuid']
            default = field['default']
            ftype = field['type']
            
            if ftype == 'string':
                f.write(f'    config_set_string("{uuid}", "{default}");\n')
            elif ftype == 'integer':
                f.write(f'    config_set_int32("{uuid}", {default});\n')
            elif ftype == 'boolean':
                val = 'true' if default else 'false'
                f.write(f'    config_set_bool("{uuid}", {val});\n')
        
        f.write('}\n')

if __name__ == '__main__':
    generate_factory_reset(sys.argv[1], sys.argv[2])
```

### Why This is BETTER Than Current System

#### Current System (Manual):
- ❌ config.h structs (C code)
- ❌ Manual HTML forms (HTML)
- ❌ Manual JSON serialization (C code)
- ❌ Manual validation (C + JS)
- ❌ 3 places to update per field

#### Optimized Option A:
- ✅ config_schema.json (ONE file)
- ✅ Auto-generated factory reset (C)
- ✅ Auto-generated UI (JavaScript)
- ✅ Schema-based validation (JavaScript)
- ✅ 1 place to update per field

#### Complexity Comparison:

| Aspect | Current | Option A Optimized |
|--------|---------|-------------------|
| Config Manager | 200+ lines | ~50 lines |
| Factory Defaults | Manual in code | Auto-generated |
| HTML Forms | Manual 100+ lines | Auto-generated |
| Validation | Duplicated C+JS | Schema-driven JS |
| Type Safety | Struct-based | API signature-based |
| JSON Parsing | None | Build-time only |
| Runtime Overhead | Low | **Lower** (simpler) |
| Lines to Add Field | ~30 lines | ~8 lines JSON |

### Cons That Were "Solved"

#### ❌ "C code loses type safety"
**✅ SOLVED:** API functions are type-safe, compiler enforces

#### ❌ "Runtime JSON parsing overhead"  
**✅ SOLVED:** No JSON parsing in runtime, only at build time

#### ❌ "More complex NVS access"
**✅ SOLVED:** Direct UUID→NVS mapping, simpler than before

#### ❌ "Harder to use config in C code"
**✅ SOLVED:** Clean API, easier than struct access

### Remaining Concerns (Minor)

1. **Build dependency:** Python 3 required
   - **Mitigation:** Already needed for ESP-IDF anyway
   
2. **Generated code in repo?**
   - **Solution:** Add to .gitignore, generate at build time
   
3. **Debugging generated code:**
   - **Mitigation:** Generated code is trivial (just set_xxx calls)

### Prototype Scope (4 hours)

**Minimal working system:**

1. Create `config_schema.json` (3 fields)
2. Write `generate_config_factory.py` (~50 lines)
3. Implement trivial config_manager (~50 lines)
4. CMake integration (~10 lines)
5. JavaScript form generator (~100 lines)
6. Test: factory reset → set values → read values

**Deliverable:** Proof that it works end-to-end

### Decision: RECOMMENDED ✅

**For ESP32 Template: Optimized Option A is PERFECT**

**Why:**
- ✅ **Simpler** than current system
- ✅ **Single source of truth**
- ✅ **No runtime complexity**
- ✅ **Better for beginners** (less code to understand)
- ✅ **Scales well** (easy to add fields)
- ✅ **Type-safe**
- ✅ **Performance** (NVS direct access)

**This is NOT over-engineering - it's simplification through automation!**

---

## Recommendation (UPDATED)

### For ESP32 Template: ~~**Keep It Simple**~~ → **Optimized Option A** ⭐

**Previous reasoning was wrong:**
- ~~Template goal: Learning resource~~ → **Simpler code IS better for learning**
- ~~Users should understand all code~~ → **Less code = easier to understand**
- ~~Manual config is explicit~~ → **Generated code is MORE explicit**
- ~~Focus on patterns~~ → **This IS the pattern to teach**

### When to Use JSON Config:

- ✅ Large applications (>20 config fields)
- ✅ Frequently changing config structure
- ✅ Non-technical users need UI config
- ✅ Multi-tenant systems
- ❌ Simple templates/examples

---

## Next Steps (When Rested 😴)

### Immediate (1 hour):
1. ✅ Create this TODO document
2. Review current config.h structure
3. Count how many fields we actually need
4. Decide: Is JSON worth it?

### If YES to JSON (Week 1):
1. Create requirements document
2. Design JSON schema format
3. Prototype with 2-3 fields
4. Measure memory impact
5. **Decision point:** Continue or revert?

### If NO to JSON (Week 1):
1. Simplify current settings page
2. Document config patterns clearly
3. Create helper macros for common tasks
4. Provide copy-paste examples

---

## Open Questions

1. **Code generation tool:** Python or Node.js?
2. **Generated code:** Commit to repo or .gitignore?
3. **Schema location:** Root, docs/, or main/components/?
4. **Validation:** Client-side only or server-side too?
5. **Testing:** How to test generated code?
6. **Versioning:** Schema version vs firmware version?
7. **Migration:** Automatic or manual NVS migration?

---

## References

- Current config: `/main/components/config_manager/config.h`
- Current settings: `/main/components/web_server/www/settings.html`
- NVS docs: ESP-IDF Non-Volatile Storage
- JSON Schema: https://json-schema.org/

---

**Last Updated:** 2025-10-28  
**Author:** Development Session Notes  
**Status:** 💤 Needs Fresh Mind & Coffee

