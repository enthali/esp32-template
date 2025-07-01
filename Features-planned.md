# ESP32 Distance Project - Current Development Roadmap

This document contains the immediate next steps for the ESP32 Distance Project. Once these are completed, we'll pick the next items from `Features-intended.md`.



## 3 Component Restructuring 📋 **NEXT**

### Step 3.1: Distance Sensor Internal Monitoring
- 📋 **Sensor Health Monitoring**: Add internal health monitoring to `distance_sensor/` component
- 📋 **Queue Overflow Tracking**: Move queue monitoring into the sensor component itself
- 📋 **Self-Contained Component**: Sensor manages its own health and performance statistics
- 📋 **Internal Monitoring Task**: Component starts its own low-priority monitoring task automatically
- 📋 **Clean Sensor API**: Remove monitoring responsibilities from main.c

**Architecture Benefits:**
- **Encapsulated Monitoring**: Distance sensor fully responsible for its own health tracking
- **Performance Statistics**: Internal tracking of queue overflows, measurement rates, and error counts
- **Simplified Integration**: Main.c no longer needs to monitor sensor health
- **Self-Healing**: Component can implement internal recovery mechanisms
- **Clean API**: Public API focused purely on distance measurement functionality

**Enhanced Distance Sensor Structure:**
```
📁 components/distance_sensor/
├── distance_sensor.h          # Public API (existing)
├── distance_sensor.c          # Core sensor driver (existing)
├── sensor_monitor.h           # Internal monitoring (new)
└── sensor_monitor.c           # Health monitoring task (new)
```

**Internal Monitoring Features:**
- Queue overflow detection and statistics
- Measurement rate tracking (target: 10Hz)
- Timeout and error rate monitoring
- Performance metrics (min/max/avg response times)
- Optional debug logging for troubleshooting

**Implementation Strategy:**
- `distance_sensor_init()` sets up internal monitoring structures (no task created)
- `distance_sensor_monitor()` function called periodically by main.c for health checks
- Monitoring is lightweight and efficient (no dedicated task overhead)
- Statistics available via optional debug API (e.g., `distance_sensor_get_stats()`)
- Main.c calls monitoring functions but doesn't need to understand the details

**Efficient Main.c with Monitoring:**
```c
void app_main(void) {
    // Hardware initialization
    led_controller_init(&led_config);
    distance_sensor_init(&distance_config);  // Sets up internal monitoring
    wifi_manager_init();                      // Still needs Step 3.2
    display_logic_init(&display_config);
    
    // Start services
    distance_sensor_start();
    wifi_manager_start();
    display_logic_start();
    
    ESP_LOGI(TAG, "Distance sensor with internal monitoring initialized");
    
    // Lightweight monitoring loop
    while(1) {
        distance_sensor_monitor();  // Lightweight health check
        wifi_manager_log_status();   // TODO: Replace with wifi_manager_monitor() in Step 3.2
        vTaskDelay(pdMS_TO_TICKS(5000));  // Monitor every 5 seconds
    }
}
```

**Deliverables:**
- Enhanced `distance_sensor/` component with internal monitoring functions
- `distance_sensor_monitor()` function for periodic health checks
- Internal sensor statistics and performance tracking
- Lightweight monitoring approach (no additional tasks)
- Optional debug API for sensor performance metrics

---

### Step 3.2: WiFi Manager Internal Monitoring
- 📋 **WiFi Health Monitoring**: Move WiFi status monitoring into `wifi_manager` component
- 📋 **Connection Status Tracking**: Internal monitoring of WiFi connection health
- 📋 **Automatic Reconnection**: Component handles connection recovery internally
- 📋 **AP Mode Management**: Self-contained fallback to AP mode logic
- 📋 **Component Restructuring**: Move `wifi_manager` to `components/wifi/` directory

**Enhanced WiFi Component Structure:**
```
📁 components/wifi/
├── wifi_manager.h             # Public API (existing)
├── wifi_manager.c             # Core WiFi management (existing)
├── wifi_monitor.h             # Internal monitoring (new)
└── wifi_monitor.c             # Connection monitoring task (new)
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

