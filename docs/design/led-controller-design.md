# LED Controller Design

## Design Traceability

| Design ID | Implements Requirement | Priority |
|-----------|------------------------|----------|
| DSN-LED-ARCH-01 | REQ-LED-1 | Mandatory |
| DSN-LED-ARCH-02 | REQ-LED-2, REQ-LED-3 | Mandatory |
| DSN-LED-API-01 | REQ-LED-2 | Mandatory |
| DSN-LED-API-02 | REQ-LED-2, REQ-LED-4 | Mandatory |
| DSN-LED-TIMING-01 | REQ-LED-1 | Mandatory |
| DSN-LED-DATA-01 | REQ-LED-4 | Mandatory |
| DSN-LED-MEM-01 | REQ-LED-3 | Mandatory |
| DSN-LED-ERR-01 | REQ-LED-2, REQ-LED-3 | Mandatory |

## Target Design Architecture

### DSN-LED-ARCH-01: RMT Peripheral Hardware Abstraction Design
Addresses: REQ-LED-1

Design: ESP32 RMT (Remote Control) peripheral abstraction for WS2812 timing generation.

- RMT Channel Configuration: 80MHz resolution, configurable GPIO pin, 64-symbol memory blocks
- Encoder Configuration: Bytes encoder with precise WS2812 timing (T0H=0.4µs, T0L=0.8µs, T1H=0.8µs, T1L=0.4µs)
- Transmission Queue: 4-deep queue for overlapping operations
- Clock Source: Default ESP32 RMT clock for consistent timing
- Channel Management: Single channel allocation with proper cleanup

Validation: RMT channel created successfully, timing parameters match WS2812 datasheet, transmission completes without errors.

### DSN-LED-ARCH-02: RAM Buffer Architecture Design
Addresses: REQ-LED-2, REQ-LED-3

Design: In-memory LED state buffer for performance optimization and atomic updates.

- Buffer Structure: Array of `led_color_t` structures (3 bytes per LED: R, G, B)
- Dynamic Allocation: `malloc()` during initialization based on configured LED count (REQ-LED-3)
- State Separation: RAM buffer independent from physical LED state until `led_show()`
- Update Pattern: Modify buffer → call `led_show()` → physical update
- Memory Management: Allocation during init, deallocation during cleanup

Validation: Buffer allocates correctly for configured LED count, updates modify only buffer until show, memory freed on cleanup.

### DSN-LED-API-01: Pixel-Level Control API Design
Addresses: REQ-LED-2

Design: Individual LED pixel manipulation with bounds checking and color utilities.

- `led_set_pixel(index, color)`: Set specific LED color with index validation
- `led_get_pixel(index)`: Read current LED color from buffer
- `led_clear_pixel(index)`: Turn off specific LED (set to black)
- Color Structure: `led_color_t` with 8-bit RGB components
- Predefined Colors: Constants for common colors (RED, GREEN, BLUE, WHITE, OFF, etc.)
- Bounds Checking: Index validation against configured LED count

Validation: Index validation prevents buffer overruns, color values stored accurately, predefined colors work correctly.

### DSN-LED-API-02: Batch Operations API Design
Addresses: REQ-LED-2, REQ-LED-4

Design: Efficient batch operations for common patterns and hardware updates.

- `led_clear_all()`: Set all LEDs to off state in single operation
- `led_show()`: Transmit complete buffer to hardware via RMT (enables REQ-LED-4)
- Color Utilities: `led_color_rgb()` constructor, `led_color_brightness()` scaling
- Status Functions: `led_get_count()`, `led_is_initialized()` for state queries
- Atomic Updates: Buffer modifications independent until `led_show()` called

Validation: Clear all zeros entire buffer, show triggers RMT transmission, utilities produce correct colors.

### DSN-LED-TIMING-01: WS2812 Timing Specification Design
Addresses: REQ-LED-1

Design: Precise WS2812 protocol timing using RMT encoder configuration.

- Bit 0 Encoding: High 0.4µs (32 ticks), Low 0.8µs (64 ticks)
- Bit 1 Encoding: High 0.8µs (64 ticks), Low 0.4µs (32 ticks)
- Reset Period: 50µs (4000 ticks) low signal between frames
- Clock Resolution: 80MHz RMT clock for 12.5ns tick precision
- MSB First: Most significant bit transmitted first per WS2812 protocol

Validation: Timing measurements match WS2812 datasheet specifications, LED strips respond correctly.

### DSN-LED-DATA-01: Color Representation and Conversion Design
Addresses: REQ-LED-4

Design: RGB color representation with GRB hardware conversion for WS2812 compatibility.

- API Color Format: RGB (Red, Green, Blue) for user-friendly interface
- Hardware Format: GRB (Green, Red, Blue) as required by WS2812 LEDs
- Conversion Logic: Reorder RGB→GRB during `led_show()` transmission preparation
- Data Buffer: Temporary allocation for GRB transmission data
- Brightness Scaling: Integer arithmetic for brightness adjustment without floating point

Validation: Color values convert correctly RGB→GRB, brightness scaling maintains color ratios, no precision loss.

### DSN-LED-MEM-01: Dynamic Memory Management Design
Addresses: REQ-LED-3

Design: Controlled dynamic allocation with proper cleanup and error handling.

- Initialization Allocation: LED buffer `malloc()` based on configured count
- Validation: LED count bounds (1-1000) to prevent excessive allocation
- Cleanup: `free()` buffer and reset pointers during deinitialization
- Temporary Allocation: GRB data buffer during transmission (freed immediately)
- Error Recovery: Cleanup partial initialization on failure

Validation: Memory allocated correctly, no leaks after deinitialization, error paths clean up properly.

### DSN-LED-ERR-01: Error Handling and Validation Design
Addresses: REQ-LED-2, REQ-LED-3

Design: Comprehensive input validation and state checking with appropriate error codes.

- State Validation: Check initialization before operations
- Bounds Checking: LED index validation against configured count (REQ-LED-3)
- Parameter Validation: Non-null config, valid LED count range
- RMT Error Handling: Propagate RMT peripheral errors to caller
- Graceful Degradation: Safe operation when not initialized (return errors, not crash)

Validation: Invalid inputs return appropriate error codes, operations fail safely, system remains stable.