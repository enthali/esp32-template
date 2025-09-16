# Distance Sensor Design

## Design Traceability

| Design ID | Implements Requirement | Priority |
|-----------|------------------------|----------|
| DSN-SNS-ARCH-01 | REQ-SNS-3, REQ-SNS-4, REQ-SNS-8 | Mandatory |
| DSN-SNS-ARCH-02 | REQ-SNS-1, REQ-SNS-2 | Mandatory |
| DSN-SNS-ISR-01 | REQ-SNS-3, REQ-SNS-8 | Critical |
| DSN-SNS-TASK-01 | REQ-SNS-4, REQ-SNS-6, REQ-SNS-10 | Mandatory |
| DSN-SNS-ALGO-01 | REQ-SNS-11 | Mandatory |
| DSN-SNS-ALGO-02 | REQ-SNS-4 | Mandatory |
| DSN-SNS-API-01 | REQ-SNS-5, REQ-SNS-7 | Mandatory |
| DSN-SNS-ERR-01 | REQ-SNS-12, REQ-SNS-13, REQ-SNS-14 | Mandatory |

## Target Design Architecture

### DSN-SNS-ARCH-01: Dual-Queue Real-Time Architecture Design
Addresses: REQ-SNS-3, REQ-SNS-4, REQ-SNS-8

Design: Separate ISR and task responsibilities using FreeRTOS queues for communication.

- ISR captures raw timestamps only, minimal processing
- Sensor task performs calculations, validation, and smoothing
- Raw queue (size 2): ISR → Task communication for timestamps
- Processed queue (size 5): Task → API communication for measurements
- Queue-based design eliminates shared variables and race conditions

Validation: ISR completes within microseconds, task receives raw timestamps, API consumers get processed measurements.

### DSN-SNS-ARCH-02: GPIO Interface Design
Addresses: REQ-SNS-1, REQ-SNS-2

Design: HC-SR04 ultrasonic sensor interface using ESP32 GPIO.

- Trigger pin (default GPIO14): Output mode, generates 10µs pulse
- Echo pin (default GPIO15): Input mode with `GPIO_INTR_ANYEDGE` interrupt
- GPIO configuration during `distance_sensor_init()` with validation
- ISR handler attached to echo pin for edge detection

Validation: GPIO pins configured correctly, ISR service installed, trigger pulse generated.

### DSN-SNS-ISR-01: Interrupt Service Routine Design
Addresses: REQ-SNS-3, REQ-SNS-8

Design: IRAM-resident ISR for deterministic timestamp capture.

- Rising edge: Capture `echo_start_time` using `esp_timer_get_time()`
- Falling edge: Capture `echo_end_time` and queue raw measurement via `xQueueSendFromISR`
- Uses `portYIELD_FROM_ISR` for task scheduling
- Marked `IRAM_ATTR` for real-time constraints
- No floating-point, heap allocation, or blocking operations

Validation: ISR execution time < 10µs, timestamps captured accurately, raw queue receives data.

### DSN-SNS-TASK-01: Sensor Task Design
Addresses: REQ-SNS-4, REQ-SNS-6, REQ-SNS-10

Design: FreeRTOS task for continuous measurement processing.

- Task priority 5, stack size 4096 bytes, pinned to core 1
- Measurement loop: trigger pulse → wait for raw queue → calculate distance → validate → smooth → enqueue
- Configurable measurement interval (default 100ms) with `vTaskDelay`
- Timeout handling for missing echo responses (default 30ms)
- Created by `distance_sensor_start()`, deleted by `distance_sensor_stop()`

Validation: Task created successfully, measurement loop operates at configured interval, processes raw data correctly.

### DSN-SNS-ALGO-01: Distance Calculation Algorithm
Addresses: REQ-SNS-11

Design: Integer arithmetic for temperature-compensated distance calculation.

- Speed of sound: `calculate_speed_of_sound_scaled(temperature_c_x10)` with scaling factor 1,000,000
- Formula: `speed = 331300000 + (606 * temperature_c_x10 * 100)`
- Distance: `distance_mm = (echo_duration_us * speed_of_sound_scaled) / 2000000`
- Avoids floating-point operations for embedded performance
- Temperature input as `temperature_c_x10` (200 = 20.0°C)

Validation: Distance calculations accurate within ±1mm for known echo durations and temperatures.

### DSN-SNS-ALGO-02: EMA Smoothing Filter Design
Addresses: REQ-SNS-4

Design: Exponential Moving Average filter using integer arithmetic.

- Formula: `smoothed = (smoothing_factor * new) + ((1000 - smoothing_factor) * previous) / 1000`
- Smoothing factor range 0-1000 (300 = 30% new, 70% previous)
- Previous value stored as `uint16_t previous_smoothed_value_mm`
- First measurement initializes filter without smoothing
- Applied only to valid measurements (not out-of-range or timeout)

Validation: Smoothing reduces noise while maintaining responsiveness, factor extremes (0, 1000) work correctly.

### DSN-SNS-API-01: Public API Design
Addresses: REQ-SNS-5, REQ-SNS-7

Design: Simple blocking and monitoring API for consumers.

- `distance_sensor_get_latest()`: Blocking receive from processed queue with `portMAX_DELAY`
- `distance_sensor_has_new_measurement()`: Non-blocking queue status check
- `distance_sensor_get_queue_overflows()`: Returns overflow counter for monitoring
- `distance_sensor_monitor()`: Health check with overflow logging
- `distance_sensor_is_running()`: Task state query

Validation: Blocking API waits for new data, non-blocking API returns immediately, monitoring functions provide accurate metrics.

### DSN-SNS-ERR-01: Error Handling Design
Addresses: REQ-SNS-12, REQ-SNS-13, REQ-SNS-14

Design: Comprehensive error detection and recovery.

- Timeout: No echo within timeout → enqueue `DISTANCE_SENSOR_TIMEOUT` with distance 0
- Out-of-range: Distance < 20mm or > 4000mm → mark `DISTANCE_SENSOR_OUT_OF_RANGE`, no smoothing
- Queue overflow: Processed queue full → drop oldest, increment `queue_overflow_counter`
- Graceful degradation with status codes in measurement structure

Validation: Error conditions produce correct status codes, overflow policy drops oldest correctly, system continues operation.
