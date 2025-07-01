# ESP32 Distance Project - Current Development Roadmap

This document contains the immediate next steps for the ESP32 Distance Project. Once these are completed, we'll pick the next items from `Features-intended.md`.



## 3 Component Restructuring 📋 **NEXT**

### Step 3.1: Distance Sensor Internal Monitoring ✅ **COMPLETED**
- ✅ **Encapsulate Monitoring Logic**: Move queue overflow monitoring from main.c into distance_sensor component
- ✅ **Simple Monitor Function**: Add `distance_sensor_monitor()` function to existing component
- ✅ **Clean Main Loop**: Replace detailed monitoring logic with simple function call
- ✅ **Minimal Changes**: Reuse existing `distance_sensor_get_queue_overflows()` API internally
- ✅ **No New Files**: Keep implementation within existing distance_sensor.c

**Architecture Benefits:**
- **Encapsulated Monitoring**: Distance sensor handles its own health monitoring internally
- **Simplified Main.c**: Main loop calls simple `distance_sensor_monitor()` function
- **Minimal Code Changes**: Reuses existing APIs and infrastructure
- **Resource Efficient**: No additional files, tasks, or complex statistics
- **Clean API**: Monitoring complexity hidden from main.c

**Simple Implementation:**
```c
// In distance_sensor.h - just add one function
esp_err_t distance_sensor_monitor(void);

// In distance_sensor.c - encapsulate existing monitoring logic
esp_err_t distance_sensor_monitor(void) {
    // Move queue overflow checking from main.c
    static uint32_t last_overflow_count = 0;
    uint32_t current_overflows = distance_sensor_get_queue_overflows();
    
    if (current_overflows > last_overflow_count) {
        ESP_LOGW(TAG, "Distance sensor queue overflows: %lu", current_overflows);
        last_overflow_count = current_overflows;
    }
    
    return ESP_OK;
}
```

**Implementation Strategy:**
- Add `distance_sensor_monitor()` function to existing distance_sensor.c
- Move queue overflow checking logic from main.c into this function
- Use existing `distance_sensor_get_queue_overflows()` API internally
- No new files, no complex statistics - just encapsulation
- Main.c calls simple monitoring function every 5 seconds

**Simplified Main.c:**
```c
void app_main(void) {
    // ...initialization...
    
    // Clean monitoring loop
    while(1) {
        distance_sensor_monitor();   // Encapsulated sensor monitoring
        wifi_manager_log_status();   // TODO: Replace in Step 3.2
        vTaskDelay(pdMS_TO_TICKS(5000));  // Monitor every 5 seconds
    }
}
```

**Deliverables Completed:**
- ✅ Simple `distance_sensor_monitor()` function in existing distance_sensor.c
- ✅ Encapsulated queue overflow monitoring logic moved from main.c
- ✅ Cleaner main.c with 5-second monitoring intervals
- ✅ Maintained `distance_sensor_is_running()` for backward compatibility
- ✅ No new files or complex statistics - minimal and pragmatic approach

---

### Step 3.2: WiFi Manager Internal Monitoring 📋 **NEXT**
- 📋 **WiFi Health Monitoring**: Move WiFi status monitoring into `wifi_manager` component
- 📋 **Connection Status Tracking**: Internal monitoring of WiFi connection health
- 📋 **Automatic Reconnection**: Component handles connection recovery internally
- 📋 **AP Mode Management**: Self-contained fallback to AP mode logic
- 📋 **Component Restructuring**: Move `wifi_manager` to `components/wifi/` directory
- 📋 **Lightweight Monitoring**: Function-based monitoring (no additional tasks)

**Enhanced WiFi Component Structure:**
```
📁 components/wifi/
├── wifi_manager.h             # Public API (existing)
├── wifi_manager.c             # Core WiFi management (existing)
├── wifi_monitor.h             # Internal monitoring (new)
└── wifi_monitor.c             # Monitoring functions (new)
```

**Internal Monitoring Features:**
- Connection status tracking and logging
- Automatic reconnection attempts with backoff
- AP mode fallback management
- Network statistics (signal strength, uptime, reconnect count)
- Smart connection retry logic

**Implementation Strategy:**
- `wifi_manager_init()` sets up internal monitoring structures (no task created)
- `wifi_manager_monitor()` function called periodically by main.c for connection health
- Component handles all connection recovery automatically within monitor function
- Lightweight monitoring approach with no dedicated task overhead
- Main.c calls simple monitoring functions without understanding internals

**Final Clean Main.c:**
```c
void app_main(void) {
    // Hardware initialization only
    led_controller_init(&led_config);
    distance_sensor_init(&distance_config);  // Sets up monitoring (Step 3.1)
    wifi_manager_init();                      // Sets up monitoring (Step 3.2)
    display_logic_init(&display_config);
    
    // Start services
    distance_sensor_start();
    wifi_manager_start();
    display_logic_start();
    
    ESP_LOGI(TAG, "All systems initialized with lightweight monitoring");
    
    // Simple, efficient monitoring loop
    while(1) {
        distance_sensor_monitor();   // Lightweight health check
        wifi_manager_monitor();      // Connection health and recovery
        vTaskDelay(pdMS_TO_TICKS(5000));  // Monitor every 5 seconds
    }
}
```

**Deliverables:**
- Restructured `components/wifi/` with internal monitoring functions
- `wifi_manager_monitor()` function for periodic connection health checks
- Self-contained WiFi connection management and recovery
- Lightweight monitoring approach (no additional tasks)
- Clean component-based architecture with efficient resource usage

