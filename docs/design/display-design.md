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

- Normal range (min ≤ distance ≤ max): Green LED at calculated position using linear interpolation
  - Formula: `led_index = (distance_mm - min_mm) * (led_count - 1) / (max_mm - min_mm)`
- Below minimum (distance < min): Red LED at position 0
- Above maximum (distance > max): Red LED at position `led_count-1`
- Boundary clamping ensures valid LED positions `[0, led_count-1]`
- Single LED illumination enforced by logic

Validation: Min distance → LED 0, max distance → LED `led_count-1`, linear interpolation between,
            below/above range → correct red LED positions.

### DSN-DSP-ALGO-02: LED Update Pattern Design (HOW to display)
Addresses: REQ-DSP-IMPL-02

Design:

- Step 1: `led_clear_all()` - set all LEDs to off state
- Step 2: `led_set_pixel(position, color)` - set desired LED from ALGO-01 decision
- Step 3: `led_show()` - transmit complete buffer to WS2812 strip

WS2812 serial protocol requires complete buffer transmission; clear-and-set pattern guarantees only one LED illuminated.

Validation: Only one LED illuminated after each update, WS2812 transmission successful.

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

### DSN-DSP-API-01: Simplified API Design
Addresses: REQ-DSP-IMPL-01

Design: Single entry point for simplified lifecycle management

- `esp_err_t display_logic_start(void)` - primary public function
- Task runs continuously until system restart
- No complex lifecycle management

Validation: Single function call starts display system, no API complexity.
