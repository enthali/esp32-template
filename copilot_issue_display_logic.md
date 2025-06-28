# GitHub Issue: Implement LED Distance Visualization System

**Title:** Feature: Implement LED Distance Visualization with Display Logic Component

**Labels:** enhancement, business-logic, architecture, led-visualization

## 🎯 Objective
Implement a complete LED distance visualization system that displays measured distances as a visual bar graph on a WS2812 LED strip.

## 📋 Requirements

### **Core Functionality**
- **Distance Range**: 10cm to 50cm mapped linearly to 40 LEDs (LED 0 to LED 39)
- **LED Spacing**: Approximately 1cm per LED (40cm range / 40 LEDs)
- **Visual Feedback**: 
  - Normal range: Green/blue gradient or solid color
  - Below 10cm: First LED red (error indicator)
  - Above 50cm: Last LED red (error indicator)
  - Sensor timeout/error: All LEDs off or specific error pattern

### **Architecture Requirements**

#### **File Structure**
```
main/
├── main.c                    # Hardware init + startup test only
├── display_logic.h/c         # NEW: Business logic component
├── CMakeLists.txt           # Update dependencies
└── test/                    # Keep for hardware testing
    ├── led_running_test.h/c  # Hardware test functions
    └── ...

components/
├── led_controller/          # Existing hardware abstraction
├── distance_sensor/         # Existing hardware abstraction
└── (keep clean component structure)
```

#### **Task Priority Hierarchy**
```
Priority 6: Distance Sensor Task  (real-time measurements - existing)
Priority 3: Display Logic Task    (NEW - LED visualization)
Priority 2: Test Task             (background only, not started by default)
Priority 1: Main Task             (coordination only)
```

### **Startup Sequence Changes**
❌ **Remove**: `test_task_start()` call from main application flow  
✅ **Add**: One-time hardware test on startup:
```c
// Run hardware test once at startup (not continuous background task)
led_running_light_test();  // Call directly, don't start task
```

### **Display Logic Task Requirements**
- **Input**: Read from distance sensor queue (non-blocking)
- **Processing**: Convert distance to LED index and color
- **Output**: Update LED strip via led_controller
- **Update Rate**: Match or slightly slower than distance sensor (1Hz)
- **Error Handling**: Visual indicators for all sensor error states

### **Mathematical Mapping**
```c
// Pseudo-code for distance mapping
if (distance < 10.0f) {
    // Error: too close - show red on first LED
    led_set_color(0, 255, 0, 0);  // Red
} else if (distance > 50.0f) {
    // Error: too far - show red on last LED  
    led_set_color(39, 255, 0, 0); // Red
} else {
    // Normal range: map 10-50cm to LEDs 0-39
    int led_index = (int)((distance - 10.0f) / 40.0f * 39.0f);
    // Set appropriate color and clear others
}
```

## 🏗️ Implementation Details

### **display_logic.h API Design**
```c
typedef struct {
    float min_distance_cm;      // 10.0f
    float max_distance_cm;      // 50.0f
    uint32_t update_interval_ms; // 1000
    // Add calibration support for future
} display_config_t;

esp_err_t display_logic_init(const display_config_t *config);
esp_err_t display_logic_start(void);
esp_err_t display_logic_stop(void);
```

### **Integration Points**
- **Distance Sensor**: Use existing `distance_sensor_get_latest()` API
- **LED Controller**: Use existing `led_set_color()`, `led_clear_all()`, `led_show()` APIs
- **Main Application**: Clean separation - main only handles initialization

### **Error Handling**
- Sensor timeout → Clear all LEDs or show error pattern
- Out of range → Red error indicators as specified
- Task creation failure → Proper error reporting
- Queue overflow → Log warning, continue operation

## 🧪 Testing Requirements
- **Hardware Test**: One-time running light on startup (verify LED strip works)
- **Range Test**: Verify correct LED activation at 10cm, 30cm, 50cm distances
- **Error Test**: Verify red indicators for out-of-range conditions
- **Performance**: Ensure smooth updates without flicker

## 📚 Design Principles
- ✅ **Clean Architecture**: Separate hardware abstraction from business logic
- ✅ **Modularity**: Reusable display_logic component
- ✅ **Real-time Safe**: Proper task priorities and non-blocking operations
- ✅ **Future Ready**: Extensible for calibration, web interface, etc.
- ✅ **ESP-IDF Best Practices**: Follow existing component patterns

## 🔗 Dependencies
- Existing `led_controller` component (hardware abstraction)
- Existing `distance_sensor` component (measurement source)  
- FreeRTOS task management
- ESP-IDF logging and error handling

## 🎯 Success Criteria
- [ ] Display logic component created with proper API
- [ ] Distance correctly mapped to LED position
- [ ] Error states visually indicated with red LEDs
- [ ] Startup hardware test runs once (no background task)
- [ ] Clean task architecture with correct priorities
- [ ] All existing functionality preserved
- [ ] Code builds and runs without issues

---

**Note**: This builds on existing working distance sensor and LED controller components. The goal is clean separation between hardware abstraction (components) and business logic (main application).

@github-copilot[bot] - This is a complex architectural implementation that combines real-time systems, LED visualization, and clean code organization. Looking forward to your solution!
