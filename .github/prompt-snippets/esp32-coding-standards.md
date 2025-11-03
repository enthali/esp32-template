# ESP32 Coding Standards

## General C Coding Guidelines

### Naming Conventions

- **Functions**: `snake_case` (e.g., `distance_sensor_init()`, `led_strip_update()`)
- **Variables**: `snake_case` (e.g., `sensor_distance`, `led_count`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_DISTANCE`, `LED_PIN`)
- **Macros**: `UPPER_SNAKE_CASE` (e.g., `#define TRIGGER_PIN 5`)
- **Structs/Enums**: `snake_case_t` (e.g., `wifi_config_t`, `sensor_data_t`)

### ESP-IDF Specific Conventions

- **Component prefixes**: Use component name as prefix (e.g., `distance_sensor_read()`)
- **ESP-IDF functions**: Follow ESP-IDF naming (e.g., `esp_wifi_init()`, `esp_http_server_start()`)
- **Error handling**: Always use `ESP_ERROR_CHECK()` for critical operations
- **Logging**: Use ESP_LOG macros with appropriate log levels

### Memory Management

```c
// Prefer stack allocation for small, temporary data
uint8_t buffer[64];

// Use ESP-IDF memory functions for dynamic allocation
char* data = (char*)malloc(size);
if (data == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory");
    return ESP_ERR_NO_MEM;
}

// Always check return values
esp_err_t ret = esp_wifi_init(&wifi_init_config);
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "WiFi init failed: %s", esp_err_to_name(ret));
    return ret;
}
```

### Error Handling Patterns

```c
// Standard ESP-IDF error handling
esp_err_t my_function(void) {
    esp_err_t ret = ESP_OK;
    
    ret = some_esp_function();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Operation failed: %s", esp_err_to_name(ret));
        goto cleanup;
    }
    
    // More operations...
    
cleanup:
    // Cleanup resources
    return ret;
}
```

### Logging Standards

```c
// Define TAG at top of file
static const char* TAG = "DISTANCE_SENSOR";

// Use appropriate log levels
ESP_LOGI(TAG, "Sensor initialized successfully");
ESP_LOGW(TAG, "Distance reading unstable: %d cm", distance);
ESP_LOGE(TAG, "Sensor communication failed");
ESP_LOGD(TAG, "Debug info: pin=%d, timeout=%d", pin, timeout);
```

### GPIO and Hardware Interface

```c
// Document pin assignments clearly
#define TRIGGER_PIN    GPIO_NUM_5
#define ECHO_PIN       GPIO_NUM_18
#define LED_DATA_PIN   GPIO_NUM_19

// Use proper GPIO configuration
gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << TRIGGER_PIN),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};
ESP_ERROR_CHECK(gpio_config(&io_conf));
```

### FreeRTOS Task Guidelines

```c
// Use proper task priorities (0-25, higher = more priority)
#define SENSOR_TASK_PRIORITY    5
#define WIFI_TASK_PRIORITY      6
#define LED_TASK_PRIORITY       4

// Use reasonable stack sizes
#define SENSOR_TASK_STACK_SIZE  2048
#define WIFI_TASK_STACK_SIZE    4096

// Proper task creation
xTaskCreate(sensor_task, "sensor_task", SENSOR_TASK_STACK_SIZE, 
            NULL, SENSOR_TASK_PRIORITY, NULL);
```

### Component Structure

```text
components/my_component/
├── CMakeLists.txt
├── include/
│   └── my_component.h
├── my_component.c
└── README.md
```

### Header File Guards

```c
#ifndef MY_COMPONENT_H
#define MY_COMPONENT_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declarations here

#ifdef __cplusplus
}
#endif

#endif // MY_COMPONENT_H
```

### Function Documentation

```c
/**
 * @brief Read distance from HC-SR04 sensor
 * 
 * This function triggers the ultrasonic sensor and measures the echo
 * time to calculate distance in centimeters.
 * 
 * @param[out] distance_cm Pointer to store the measured distance
 * @return ESP_OK on success, ESP_ERR_TIMEOUT on sensor timeout,
 *         ESP_ERR_INVALID_ARG if distance_cm is NULL
 */
esp_err_t distance_sensor_read(uint16_t* distance_cm);
```

### File Documentation

```c
/**
 * @file distance_sensor.c
 * @brief HC-SR04 ultrasonic distance sensor implementation
 * 
 * This module provides functions to initialize and read from an HC-SR04
 * ultrasonic distance sensor connected to ESP32 GPIO pins.
 * 
 * @author ESP32 Distance Project
 * @date 2025
 */
```

**Documentation Requirements:**

- **Every function must have a Doxygen comment** explaining purpose, parameters, and return values
- **Every file must have a header comment** describing the module's purpose
- **Complex algorithms must include inline comments** explaining the logic
- **Hardware dependencies must be documented** (pin assignments, timing constraints)

### Configuration Management

```c
// Use Kconfig for configurable parameters
#ifdef CONFIG_DISTANCE_SENSOR_MAX_RANGE_CM
#define MAX_SENSOR_RANGE CONFIG_DISTANCE_SENSOR_MAX_RANGE_CM
#else
#define MAX_SENSOR_RANGE 400  // Default value
#endif
```

## ESP32-Specific Best Practices

### IRAM Usage

```c
// Only use IRAM_ATTR for time-critical interrupt handlers
IRAM_ATTR void gpio_isr_handler(void* arg) {
    // Keep ISR code minimal and fast
}
```

### WiFi Event Handling

```c
// Proper event handler registration
ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                          &wifi_event_handler, NULL));
ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                          &ip_event_handler, NULL));
```

### Memory Efficiency

- Use `const` for read-only data to place it in flash
- Use appropriate data types (`uint8_t` vs `uint32_t`)
- Consider using packed structs for wire protocols
- Free allocated memory promptly

### Component Dependencies

```cmake
# In CMakeLists.txt
idf_component_register(
    SRCS "src/my_component.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_wifi esp_http_server
    PRIV_REQUIRES esp_timer
)
```

## Documentation Standards

### Sphinx-Needs Documentation Methodology

This project uses **Sphinx-Needs** for professional requirements and design documentation:

#### Requirements Documentation

Requirements are documented in reStructuredText (`.rst`) files in `docs/11_requirements/`:

```rst
.. req:: Configuration Manager Initialization
   :id: REQ_CFG_1
   :links: REQ_SYS_CFG_1
   :status: approved
   :priority: mandatory
   :tags: config, initialization

   **Description:** The system SHALL initialize configuration manager on boot.

   **Acceptance Criteria:**
   - AC-1: Configuration loaded from NVS within 500ms
   - AC-2: Default values used if NVS is empty
```

#### Design Specifications

Design documents are maintained separately in `docs/12_design/` to avoid code bloat:

- Detailed architectural decisions
- Component interaction diagrams  
- Algorithm specifications
- State machine definitions

Design specs link to requirements using `:links:` attribute for traceability.

#### Code-Level Traceability

Code should reference requirement and design IDs in comments:

```c
/**
 * @file config_manager.c
 * @brief Configuration management implementation
 * 
 * REQUIREMENTS TRACEABILITY:
 * - REQ_CFG_1: Configuration initialization
 * - REQ_CFG_3: Runtime configuration structure
 * - REQ_CFG_4: NVS persistent storage
 */

/**
 * @brief Initialize configuration manager - implements REQ_CFG_1
 * 
 * Loads configuration from NVS or uses defaults if not found.
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t config_manager_init(void) {
    // Implementation
}
```

#### Benefits of Sphinx-Needs

- ✅ **Separate concerns**: Design docs don't bloat code files
- ✅ **Professional tooling**: Auto-generated traceability matrices and graphs
- ✅ **Version controlled**: Design docs tracked in Git like code
- ✅ **Bidirectional links**: Requirements ↔ Design ↔ Code
- ✅ **Scalable**: Works for projects of any size
