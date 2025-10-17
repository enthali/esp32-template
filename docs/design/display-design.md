# Display Design

## Design Traceability

| Design ID | Implements Requirement | Priority |
|-----------|------------------------|----------|
| DSN-DSP-OVERVIEW-01 | REQ-DSP-OVERVIEW-01 | Mandatory |
| DSN-DSP-ARCH-01 | REQ-DSP-IMPL-01 | Mandatory |
| DSN-DSP-ARCH-02 | REQ-DSP-OVERVIEW-02 | Mandatory |
| DSN-DSP-ALGO-01 | REQ-DSP-IMPL-03, REQ-DSP-VISUAL-01, REQ-DSP-VISUAL-02, REQ-DSP-VISUAL-03, REQ-DSP-VISUAL-04 | Mandatory |
| DSN-DSP-ALGO-02 | REQ-DSP-IMPL-02 | Mandatory |
| DSN-DSP-ALGO-03 | REQ-SYS-1 | Mandatory |
| DSN-DSP-ANIM-01 | REQ-DSP-ANIM-01, REQ-DSP-ANIM-02, REQ-DSP-ANIM-04 | Mandatory |
| DSN-DSP-ANIM-02 | REQ-DSP-ANIM-03, REQ-DSP-ANIM-04 | Mandatory |
| DSN-DSP-ANIM-03 | REQ-DSP-ANIM-05, REQ-DSP-VISUAL-03 | Mandatory |
| DSN-DSP-ANIM-04 | REQ-DSP-ANIM-04 | Mandatory |
| DSN-DSP-API-01 | REQ-DSP-IMPL-01 | Mandatory |

## Target Design Architecture

### DSN-DSP-OVERVIEW-01: WS2812 Hardware Integration Design

Addresses: REQ-DSP-OVERVIEW-01

Design: WS2812 addressable LED strip as primary display hardware with config-driven parameters.

- LED count: Configurable via `config_manager` API (1-100 LEDs, validated)
- Brightness: Configurable via `config_manager` API (0-255, hardware PWM control)
- GPIO pin: Hardware-specific configuration for WS2812 data line
- Initialization: LED hardware initialized before task starts processing measurements
- Real-time operation: Continuous reactive updates as distance measurements arrive

Validation: LED strip responds to configuration changes, hardware initialization successful.

### DSN-DSP-ARCH-01: Task-Based Architecture Design

Addresses: REQ-DSP-IMPL-01

Design: Implement single FreeRTOS task that blocks on `distance_sensor_get_latest()`.

- Task priority set below measurement task to ensure proper data flow hierarchy
- Task runs continuously until system restart
- Core assignment and stack size from centralized configuration
- Blocking wait pattern eliminates polling overhead and provides immediate response

Validation: Task created successfully, priority hierarchy maintained, blocks efficiently.

### DSN-DSP-ARCH-02: Configuration Integration Design

Addresses: REQ-DSP-OVERVIEW-02, REQ-CFG-2

Design: Use `config_manager` API for all distance range parameters.

- Obtain min/max distance values via `config_get_current()` and use `distance_min_mm` / `distance_max_mm` fields (millimeters)
- Cache config values locally at task startup for performance
- Configuration changes handled via system restart (restart-based architecture)
- Configuration validation responsibility belongs to `config_manager`

Validation: All distance parameters obtained from `config_manager` API, no separate config structures.

### DSN-DSP-ALGO-01: Distance-to-Visual Mapping Algorithm (WHAT to display)

Addresses: REQ-DSP-IMPL-03, REQ-DSP-VISUAL-01/02/03/04

Design:

- **Zone Calculation** (configuration-independent, integer math):
  - `ideal_size = (led_count * 10) / 100` (10% of strip)
  - `ideal_center = (led_count * 30) / 100` (30% position)
  - `ideal_start = ideal_center - (ideal_size / 2)`
  - `ideal_end = ideal_start + ideal_size - 1`
  - Example for 40 LEDs: ideal_center=12, ideal_size=4, ideal_start=10, ideal_end=13

- **Normal range** (min ≤ distance ≤ max): Dual-layer display with zone-based behavior
  - Position Formula: `led_index = (distance_mm - min_mm) * (led_count - 1) / (max_mm - min_mm)`
  - Zone 1 "too close" (led_index < ideal_start): Orange position LED (50% brightness) + red animation (100% brightness) (see DSN-DSP-ANIM-01)
  - Zone 2 "ideal" (ideal_start ≤ led_index ≤ ideal_end): All ideal zone LEDs red (see DSN-DSP-ANIM-02)
  - Zone 3 "too far" (led_index > ideal_end): Green position LED + white animation (see DSN-DSP-ANIM-01)

- **Below minimum** (distance < min): Emergency blinking pattern (see DSN-DSP-ANIM-03)
- **Above maximum** (distance > max): Blue "too far" animation only (no position indicator)
- **Boundary clamping**: Ensures valid LED positions `[0, led_count-1]`

Validation: Zone boundaries calculated correctly for 20-100 LED configurations, position mapping linear,
           animations trigger in correct zones, ideal zone calculation centered properly.

### DSN-DSP-ALGO-02: Multi-Layer Rendering Pipeline (HOW to display)

Addresses: REQ-DSP-IMPL-02, REQ-DSP-ANIM-04

Design: Frame-based rendering with priority-based layer compositing

**Rendering Pipeline** (executed at 10 FPS / 100ms intervals):

1. **Clear**: `led_clear_all()` - reset all LEDs to off state
2. **Ideal Zone Background Layer** (lowest priority, 2% brightness):
   - Always render ideal zone LEDs (ideal_start through ideal_end) at 2% red brightness
   - Provides constant visual reference of target parking zone
   - Visible in Zone 1 (too close) and Zone 3 (too far)
3. **Animation Layer** (2-100% brightness depending on zone):
   - If in "too far" zone: Render white animation LED at current animation position (2% brightness)
   - If in "too close" zone: Render red animation LED at current animation position (100% brightness)
   - If out of range (above max): Render white animation LED only (2% brightness)
4. **Position Layer** (overlay, 50-100% brightness):
   - If in "too close" zone: Render orange LED at measured position (50% brightness)
   - If in "too far" zone: Render green LED at measured position (100% brightness)
   - Position LED overwrites animation LED if at same position (higher brightness dominates)
5. **Ideal Zone Full Brightness Layer** (override):
   - If in ideal zone: Overwrite ALL ideal zone LEDs with red at 100% brightness (ideal_start through ideal_end)
   - No position or animation shown (entire zone is valid)
   - Brightness increase from 2% → 100% provides positive feedback
6. **Emergency Layer** (highest priority):
   - If below minimum: Overwrite LEDs 0, 10, 20, 30... with blinking red (500ms ON/OFF)
   - Emergency pattern overrides all other layers
7. **Commit**: `led_show()` - transmit complete buffer to WS2812 strip atomically

**Timing Architecture**:

- FreeRTOS timer callback at 100ms intervals for animation updates
- Timer increments animation position and toggles blink state
- Distance measurement updates position immediately (no waiting for timer)
- Non-blocking: Timer callback completes in <1ms

Validation: All layers composite correctly, priority enforced, single led_show() per frame,
           no visual tearing, animation smooth at 10 FPS, ideal zone always visible as background.

### DSN-DSP-ALGO-03: Embedded Arithmetic Architecture Design

Addresses: REQ-SYS-1

Design: Pure integer arithmetic for all distance calculations and display operations.

- Distance representation: `uint16_t` millimeters (0-65535mm)
- Position calculations: Multiplication before division for precision preservation
- Boundary checks: Integer comparisons
- Memory efficiency: 2-byte integers vs 4-byte floats
- Execution speed: Integer ALU operations vs FPU operations
- Deterministic timing: No floating-point precision variations

Rationale: Avoid floating-point on resource-constrained microcontrollers unless necessary.

Validation: All arithmetic operations complete within deterministic time bounds.

### DSN-DSP-ANIM-01: Directional Animation Design

Addresses: REQ-DSP-ANIM-01, REQ-DSP-ANIM-02, REQ-DSP-ANIM-04

Design: Running LED animation for directional guidance

**Animation State Machine**:

```c
typedef struct {
    uint8_t current_position;    // Current animation LED position
    bool animation_active;       // Is animation running?
    bool animation_direction;    // true = forward (too close), false = backward (too far)
    uint8_t animation_start;     // Start position for animation
    uint8_t animation_end;       // End position for animation
} animation_state_t;
```

**Animation Behavior**:

- **Too Far Zone** (position > ideal_end):
  - Start: LED position `led_count - 1` (far end)
  - End: LED position `ideal_end` (ideal zone boundary)
  - Direction: Backward (toward ideal zone)
  - Color: White at 2% brightness (RGB: ~5, ~5, ~5)
  - Loop: When reaching ideal_end, restart from led_count-1

- **Too Close Zone** (position < ideal_start):
  - Start: LED position 0 (near end)
  - End: LED position `ideal_start` (ideal zone boundary)
  - Direction: Forward (toward ideal zone)
  - Color: Red at 100% brightness (RGB: 255, 0, 0)
  - Loop: When reaching ideal_start, restart from 0

- **Animation Update** (100ms timer callback):
  - Increment/decrement current_position based on direction
  - Wrap around when reaching end position
  - Update happens independent of distance measurement updates

**Color Calculation**:

```c
// White (too far): led_color_brightness(LED_COLOR_WHITE, 5)  → (~5, ~5, ~5)
// Red (too close): led_color_brightness(LED_COLOR_RED, 255) → (255, 0, 0)
```

**Position Indicator Brightness** (too close zone only):

```c
// Orange position at 50%: led_color_brightness(LED_COLOR_ORANGE, 128) → (128, 42, 0)
```

Validation: Animation smooth, loops correctly, stops when zone changes, color brightness correct.

### DSN-DSP-ANIM-02: Ideal Zone Display Design

Addresses: REQ-DSP-ANIM-03, REQ-DSP-ANIM-04

Design: Dual-brightness red indication for ideal parking zone providing constant visual reference and positive feedback

**Ideal Zone Background Rendering** (always active):

- Loop through LEDs from ideal_start to ideal_end
- Set each LED to red at 2% brightness (~5, 0, 0)
- Renders as base layer (Step 2 in rendering pipeline)
- Provides constant visual target reference visible from any distance
- Can be overwritten by higher priority layers (animation, position, full brightness ideal zone)

**Ideal Zone Full Brightness Rendering** (when in zone):

- When `ideal_start ≤ led_index ≤ ideal_end`:
  - Loop through LEDs from ideal_start to ideal_end
  - Set each LED to solid red (255, 0, 0) at 100% brightness
  - Overwrite any animation or position layer LEDs in this range
  - No animation running
  - No position indicator (entire zone valid)

**Zone Persistence**:

- Background ideal zone (2%) visible at all times
- Full brightness ideal zone (100%) persists while position remains in zone
- Brightness increase 2% → 100% provides immediate positive feedback when entering zone
- Immediately switches to position+animation when exiting zone
- No hysteresis needed (clear boundary conditions)

Validation: Background ideal zone always visible at 2%, full brightness overrides other layers when in zone,
           immediate brightness transition provides clear feedback, no visual glitches.

### DSN-DSP-ANIM-03: Emergency Blinking Pattern Design

Addresses: REQ-DSP-ANIM-05, REQ-DSP-VISUAL-03

Design: 1 Hz blinking pattern on every 10th LED for emergency warning

**Blink State Machine**:

```c
typedef struct {
    bool blink_state;           // true = ON, false = OFF
    uint32_t last_toggle_ms;    // Last toggle timestamp
} blink_state_t;
```

**Blink Behavior**:

- **Timing**: Toggle every 500ms using FreeRTOS tick count
  - `if (current_time_ms - last_toggle_ms >= 500) { toggle blink_state }`
  
- **LED Positions**: Every 10th position (0, 10, 20, 30, ...)
  - Loop: `for (int i = 0; i < led_count; i += 10)`
  
- **ON State** (blink_state = true):
  - LEDs 0, 10, 20, 30... = Red (255, 0, 0)
  
- **OFF State** (blink_state = false):
  - LEDs 0, 10, 20, 30... = Off (0, 0, 0)

- **Priority**: Highest - overwrites all other layers when distance < min

**Layer Suppression During Emergency**:

- When emergency active (`distance < min`), skip rendering of:
  - Ideal zone background layer (2% red)
  - Animation layer (directional guidance)
  - Position layer (current location)
  - Ideal zone full brightness layer (not reachable during emergency)
- Only emergency blink pattern renders
- Clear visual focus: ONLY the danger warning, no distractions

**Emergency Exit**:

- When distance returns to valid range, resume normal layer rendering
- Ideal zone background becomes visible again
- Immediate transition, no hysteresis

Validation: Blink frequency accurate (1 Hz ± 10%), pattern positions correct, overrides all layers,
           ideal zone background suppressed during emergency, clean transition when exiting emergency.

### DSN-DSP-ANIM-04: Frame-Based Timing Architecture

Addresses: REQ-DSP-ANIM-04

Design: FreeRTOS timer for non-blocking animation updates

**Timer Configuration**:

```c
esp_timer_handle_t animation_timer;
esp_timer_create_args_t timer_args = {
    .callback = animation_timer_callback,
    .arg = NULL,
    .dispatch_method = ESP_TIMER_TASK,  // Run in timer task context
    .name = "anim_timer"
};
esp_timer_create(&timer_args, &animation_timer);
esp_timer_start_periodic(animation_timer, 100000);  // 100ms = 100000µs
```

**Timer Callback** (100ms intervals):

1. Get current distance measurement (cached from display task)
2. Update animation state (increment position, toggle blink)
3. Render frame using dual-layer pipeline (DSN-DSP-ALGO-02)
4. Return immediately (non-blocking, <1ms execution time)

**Synchronization**:

- Distance measurement updates shared variable (atomic read/write on ESP32)
- Timer task priority set lower than display task
- No mutex needed (simple state machine, single writer)

**Memory Overhead**:

- animation_state_t: ~6 bytes
- blink_state_t: ~8 bytes
- Timer handle: ~4 bytes
- Total: ~18 bytes additional RAM

Validation: Timer accuracy ±5ms, callback execution <1ms, no blocking, animation smooth.

### DSN-DSP-API-01: Simplified API Design

Addresses: REQ-DSP-IMPL-01

Design: Single entry point for simplified lifecycle management

- `esp_err_t display_logic_start(void)` - primary public function
- Task runs continuously until system restart
- No complex lifecycle management

Validation: Single function call starts display system, no API complexity.
