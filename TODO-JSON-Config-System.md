# JSON-Based Configuration System

**Status:** 🚀 Implementation Phase  
**Branch:** `feature/json-config-system`  
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

## Solution: Optimized Option A

**Architecture:** Single JSON schema → Build-time code generation → Type-safe C + Dynamic UI

### Key Insight: Eliminate Runtime JSON Parsing

- ✅ `config_schema.json` is source of truth
- ✅ Python script generates factory defaults at build time
- ✅ No JSON parsing in C runtime (only direct NVS access)
- ✅ Browser loads schema for dynamic UI generation
- ✅ Type-safe C API via function signatures

### Example Schema

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
      "description": "General application configuration",
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
      "key": "led_count",
      "type": "integer",
      "label": "Number of LEDs",
      "default": 60,
      "required": true,
      "min": 1,
      "max": 500,
      "step": 1,
      "group": "application",
      "order": 1
    }
  ]
}
```

---

## Architecture: Build-Time Code Generation

```text
config_schema.json (source of truth)
    ↓
Python Script (build-time)
    ↓
config_factory_generated.c (compiled into firmware)
    ↓
Runtime: NVS access only (no JSON parsing!)
```

### Why This Works

1. **Type Safety** - C API functions enforce types by signature
2. **No Runtime Overhead** - JSON parsing happens only at build time
3. **Simplicity** - Config manager is thin wrapper around NVS
4. **Dynamic UI** - Browser loads schema for form generation

---

## Design Decisions

### Key Strategy

Use `key` field directly as NVS key:

```c
const char* ssid = config_get_string("wifi_ssid");
config_set_string("wifi_ssid", "MyNetwork");
```

**Benefits:**

- Saves ~15 bytes per field
- Simpler C code
- NVS keys ≤15 characters

### Supported Types

- `string` - text inputs
- `password` - password inputs
- `integer` - number inputs
- `boolean` - checkboxes
- `hidden` - hidden inputs

### C API

```c
esp_err_t config_init(void);
esp_err_t config_factory_reset(void);

esp_err_t config_get_string(const char* key, char* buf, size_t len);
esp_err_t config_get_int32(const char* key, int32_t* value);
esp_err_t config_get_bool(const char* key, bool* value);

esp_err_t config_set_string(const char* key, const char* value);
esp_err_t config_set_int32(const char* key, int32_t value);
esp_err_t config_set_bool(const char* key, bool value);

void config_write_factory_defaults(void);  // Generated
```

---

## Python Generator

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
```

---

## Build Integration

CMakeLists.txt:

```cmake
# Generate factory defaults from schema
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config_factory_generated.c
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_config_factory.py
            ${CMAKE_CURRENT_SOURCE_DIR}/config_schema.json
            ${CMAKE_CURRENT_BINARY_DIR}/config_factory_generated.c
    DEPENDS config_schema.json tools/generate_config_factory.py
)

idf_component_register(
    SRCS "config_manager.c"
         "${CMAKE_CURRENT_BINARY_DIR}/config_factory_generated.c"
    INCLUDE_DIRS "include"
    EMBED_FILES "config_schema.json"
)
```

---

## Web UI Generation

Browser fetches schema and generates form dynamically:

```javascript
async function initializeSettings() {
    const response = await fetch('/config_schema.json');
    const schema = await response.json();
    generateFormFromSchema(schema);
}

function generateFormFromSchema(schema) {
    for (const group of schema.groups) {
        const groupDiv = document.createElement('div');
        groupDiv.innerHTML = `<h2>${group.label}</h2>
                              <p>${group.description}</p>`;
        
        for (const field of schema.fields) {
            if (field.group === group.id) {
                const input = createInputElement(field);
                groupDiv.appendChild(input);
            }
        }
        
        document.getElementById('settings').appendChild(groupDiv);
    }
}
```

---

## Best Practices

### Key Naming Convention

Use `snake_case` for all keys, matching C variable naming:

```json
{
  "key": "led_count",        // C code: config_get_int32("led_count", &val)
  "key": "wifi_ssid",        // C code: config_get_string("wifi_ssid", buf, len)
  "key": "update_interval_ms" // C code: config_get_int32("update_interval_ms", &val)
}
```

**Benefit:** Consistency between JSON schema and C code makes it obvious when they match.

### Type Safety (Without Runtime Parsing)

The system does NOT parse JSON at runtime in C, so **type mismatches will crash**:

```c
// ❌ WRONG - Type mismatch causes runtime error
config_get_int32("wifi_ssid", &value);  // wifi_ssid is type="string"!

// ✅ CORRECT - Types must match
char ssid[33];
config_get_string("wifi_ssid", ssid, sizeof(ssid));

int32_t count;
config_get_int32("led_count", &count);  // led_count is type="integer"
```

**This is intentional:** For a template, explicit code is better than magic. Type errors are caught immediately during testing.

### Validation: Schema vs Code

| Location | Validates |
|----------|-----------|
| `config_schema.json` | Field definitions, UI generation |
| Browser | Form input (min/max, required, pattern) |
| C Code | Type must match schema type |
| Testing | Integration verification |

**Important:** The server (C code) does NOT re-validate! It trusts the browser. For production systems, add server-side validation.

### Adding a New Config Field

1. **Add to schema:**

   ```json
   {
     "key": "my_setting",
     "type": "integer",
     "label": "My Setting",
     "default": 100,
     "min": 1,
     "max": 1000,
     "group": "application",
     "order": 1
   }
   ```

2. **Use in code:**

   ```c
   int32_t my_setting;
   esp_err_t err = config_get_int32("my_setting", &my_setting);
   if (err != ESP_OK) {
       ESP_LOGW(TAG, "Failed to read my_setting: %s", esp_err_to_name(err));
       my_setting = 100;  // Use default
   }
   ```

3. **Verify:**
   - Settings UI auto-generates from schema ✅
   - Web form shows validation rules (min/max) ✅
   - Reload web UI to test

### Optional: Validator Script (Nice-to-Have)

For projects with many config fields, add a pre-build validator:

```python
#!/usr/bin/env python3
# tools/validate_config_schema.py
import json
import re

def validate_schema(schema_file, source_files):
    """Check that all get_xxx("key") calls match schema types"""
    with open(schema_file) as f:
        schema = json.load(f)
    
    # Build key→type mapping
    key_types = {field['key']: field['type'] for field in schema['fields']}
    
    # Search for config_get_xxx("key") in C files
    for src_file in source_files:
        with open(src_file) as f:
            for line_num, line in enumerate(f, 1):
                # Find config_get_xxx("key") patterns
                match = re.search(r'config_get_(\w+)\s*\(\s*"(\w+)"', line)
                if match:
                    func_type = match.group(1)  # int32, string, bool
                    key = match.group(2)
                    
                    if key not in key_types:
                        print(f"ERROR: {src_file}:{line_num} - Key '{key}' not in schema")
                    elif not type_matches(func_type, key_types[key]):
                        print(f"ERROR: {src_file}:{line_num} - Type mismatch for '{key}': " +
                              f"got {func_type}, expected {key_types[key]}")

def type_matches(func_type, schema_type):
    """Check if function type matches schema type"""
    mapping = {
        'int32': 'integer',
        'bool': 'boolean',
        'string': ('string', 'password'),
    }
    return schema_type in mapping.get(func_type, ())

if __name__ == '__main__':
    import glob
    schema = 'main/components/config_manager/config_schema.json'
    sources = glob.glob('main/**/*.c', recursive=True)
    validate_schema(schema, sources)
```

**Usage:** Add to build pre-step to catch type mismatches before compilation.

---

## Comparison

| Aspect | Current | New |
|--------|---------|-----|
| Config Manager | 200+ lines | ~50 lines |
| Factory Defaults | Manual code | Auto-generated |
| HTML Forms | Manual 100+ lines | Auto-generated |
| Lines to Add Field | ~30 | ~8 (JSON) |

---

## Flash Savings

Per-field: ~65 bytes saved (flat validation, direct keys)

For 10 fields: ~650 bytes saved

---

## Implementation Plan

### Phase 1: Prototype (4 hours)

1. Create minimal `config_schema.json` (2-3 fields)
2. Write Python generator (~50 lines)
3. Implement basic config manager (~50 lines)
4. Create simple form generator (~100 lines)
5. Test factory reset end-to-end

### Phase 2: Full Implementation (8 hours)

1. Migrate all config fields to schema
2. Complete config manager API
3. Update all endpoints
4. Enhanced form generation
5. Add factory reset endpoint

### Phase 3: Testing & Documentation (4 hours)

1. Test all config fields
2. Verify NVS persistence
3. Browser compatibility testing
4. Write user guide for adding fields
5. Create examples

---

## Next Steps

1. **Generated code:** Store in `.gitignore`, generate at build time
2. **Schema location:** `main/components/config_manager/config_schema.json`
3. **Python script:** `tools/generate_config_factory.py`
4. **Validation:** Browser validates, server trusts
5. **Migration:** Keep NVS keys unchanged for compatibility
