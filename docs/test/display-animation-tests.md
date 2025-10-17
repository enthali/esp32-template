# Display Animation System Test Specification

**Document ID**: TST-DSP-ANIM-ESP32-DISTANCE  
**Version**: 1.0  
**Date**: 2025-10-17  
**Author**: ESP32 Distance Project Team  
**Requirements Traceability**: REQ-DSP-ANIM-01 through REQ-DSP-ANIM-05, REQ-DSP-VISUAL-02/03/04  
**Design Traceability**: DSN-DSP-ANIM-01 through DSN-DSP-ANIM-04  

## Test Overview

This document specifies test cases for the Dual-Layer LED Display Animation System, ensuring compliance with requirements for directional animations, ideal zone indication, and emergency proximity warnings.

## Test Categories

- **Unit Tests**: Individual function validation (zone calculation, animation state)
- **Integration Tests**: Component interaction validation (timer, display, sensor)  
- **System Tests**: End-to-end functionality validation (all zones, transitions)
- **Performance Tests**: Real-time requirements validation (10 FPS, timing accuracy)
- **Visual Tests**: Manual observation validation (colors, brightness, patterns)

---

## Phase 1: Zone Calculation Tests

### TST-DSP-ANIM-1: Configuration-Independent Zone Calculation

**Covers**: REQ-DSP-VISUAL-02 AC-8, DSN-DSP-ALGO-01  
**Type**: Unit Test  
**Priority**: Mandatory  

**Test Objective**: Verify zone boundaries calculated correctly for various LED configurations

#### TST-DSP-ANIM-1.1: Zone Calculation for 40 LEDs

**Test Steps**:

1. Initialize LED controller with 40 LEDs
2. Call calculate_zones(40, &zones)
3. Verify ideal_center = 12 (30% of 40)
4. Verify ideal_size = 4 (10% of 40)
5. Verify ideal_start = 10 (center - size/2)
6. Verify ideal_end = 13 (start + size - 1)

**Expected Results**:

- ideal_center: 12
- ideal_size: 4
- ideal_start: 10
- ideal_end: 13
- Zone ranges: Too close [0-9], Ideal [10-13], Too far [14-39]

#### TST-DSP-ANIM-1.2: Zone Calculation for Various LED Counts

**Test Steps**:

1. Test with LED counts: 20, 40, 60, 100
2. For each count, verify:
   - ideal_size = (led_count * 10) / 100
   - ideal_center = (led_count * 30) / 100
   - ideal_start = ideal_center - (ideal_size / 2)
   - ideal_end = ideal_start + ideal_size - 1
3. Verify zones non-overlapping
4. Verify zones span entire LED strip

**Expected Results**:

- 20 LEDs: ideal zone [5-6], size=2, center=6
- 40 LEDs: ideal zone [10-13], size=4, center=12
- 60 LEDs: ideal zone [15-20], size=6, center=18
- 100 LEDs: ideal zone [25-34], size=10, center=30
- All calculations use integer arithmetic only
- No gaps or overlaps between zones

---

## Phase 2: Animation State Machine Tests

### TST-DSP-ANIM-2: Animation State Transitions

**Covers**: REQ-DSP-ANIM-01, REQ-DSP-ANIM-02, DSN-DSP-ANIM-01  
**Type**: Unit Test  
**Priority**: Mandatory  

#### TST-DSP-ANIM-2.1: "Too Far" Animation Activation

**Test Steps**:

1. Set LED position to 20 (beyond ideal_end=13)
2. Call update_animation_state(20, 40)
3. Verify anim_state.active = true
4. Verify anim_state.direction_forward = false
5. Verify anim_state.start_pos = 39
6. Verify anim_state.end_pos = 13
7. Verify anim_state.current_position = 39

**Expected Results**:

- Animation activates for "too far" zone
- Direction set to backward (39 → 13)
- Initial position at far end (LED 39)

#### TST-DSP-ANIM-2.2: "Too Close" Animation Activation

**Test Steps**:

1. Set LED position to 5 (before ideal_start=10)
2. Call update_animation_state(5, 40)
3. Verify anim_state.active = true
4. Verify anim_state.direction_forward = true
5. Verify anim_state.start_pos = 0
6. Verify anim_state.end_pos = 10
7. Verify anim_state.current_position = 0

**Expected Results**:

- Animation activates for "too close" zone
- Direction set to forward (0 → 10)
- Initial position at near end (LED 0)

#### TST-DSP-ANIM-2.3: Ideal Zone Animation Deactivation

**Test Steps**:

1. Activate "too far" animation (position = 20)
2. Move position to ideal zone (position = 11)
3. Call update_animation_state(11, 40)
4. Verify anim_state.active = false

**Expected Results**:

- Animation deactivates when entering ideal zone
- No animation runs while in ideal zone

### TST-DSP-ANIM-3: Animation Position Updates

**Covers**: REQ-DSP-ANIM-01 AC-4/5, REQ-DSP-ANIM-02 AC-4/5, DSN-DSP-ANIM-01  
**Type**: Unit Test  
**Priority**: Mandatory  

#### TST-DSP-ANIM-3.1: Forward Animation Loop

**Test Steps**:

1. Set up "too close" animation (0 → 10)
2. For i = 0 to 11:
   - Call advance_animation()
   - Record current_position
3. Verify positions: 0, 1, 2, ..., 9, 0, 1 (loops back)

**Expected Results**:

- Animation advances 0 → 1 → 2 → ... → 9
- Loops back to 0 after reaching end_pos (10)
- Continuous smooth animation

#### TST-DSP-ANIM-3.2: Backward Animation Loop

**Test Steps**:

1. Set up "too far" animation (39 → 13)
2. For i = 0 to 27:
   - Call advance_animation()
   - Record current_position
3. Verify positions: 39, 38, 37, ..., 14, 13, 39 (loops back)

**Expected Results**:

- Animation advances 39 → 38 → 37 → ... → 14 → 13
- Loops back to 39 after reaching end_pos (13)
- Continuous smooth animation

---

## Phase 3: Dual-Layer Rendering Tests

### TST-DSP-ANIM-4: Layer Compositing

**Covers**: REQ-DSP-ANIM-04, DSN-DSP-ALGO-02  
**Type**: Integration Test  
**Priority**: Mandatory  

#### TST-DSP-ANIM-4.1: Position Over Animation Layer

**Test Steps**:

1. Set position = 5 (too close zone)
2. Set animation position = 5 (same as measured position)
3. Call render_frame()
4. Read LED 5 color
5. Verify color is GREEN (position layer), not orange (animation layer)

**Expected Results**:

- Position layer (green, 100% brightness) visible
- Animation layer (orange, 2% brightness) overridden
- Layer priority: Position > Animation

#### TST-DSP-ANIM-4.2: Ideal Zone Over All Layers

**Test Steps**:

1. Set position = 11 (ideal zone)
2. Set animation active (should be ignored)
3. Call render_frame()
4. Read LEDs 10, 11, 12, 13 (ideal zone)
5. Verify all are RED (100% brightness)

**Expected Results**:

- All ideal zone LEDs (10-13) show red
- Position layer not visible
- Animation layer not visible
- Layer priority: Ideal Zone > Position > Animation

#### TST-DSP-ANIM-4.3: Emergency Over All Layers

**Test Steps**:

1. Set distance below minimum (emergency condition)
2. Set blink_state.blink_on = true
3. Call render_frame()
4. Read LEDs 0, 10, 20, 30
5. Verify all are RED (100% brightness)
6. Read LEDs 1, 11, 21, 31
7. Verify all are OFF

**Expected Results**:

- LEDs 0, 10, 20, 30 show red (emergency pattern)
- Other LEDs off
- Layer priority: Emergency > all others

### TST-DSP-ANIM-5: Color and Brightness Validation

**Covers**: REQ-DSP-ANIM-01 AC-2, REQ-DSP-ANIM-02 AC-2  
**Type**: Visual Test  
**Priority**: Mandatory  

#### TST-DSP-ANIM-5.1: Animation Brightness (2%)

**Test Steps**:

1. Activate "too far" animation (blue)
2. Measure LED brightness at animation position
3. Verify RGB values approximately (0, 0, 5) for blue at 2%
4. Activate "too close" animation (orange)
5. Verify RGB values approximately (5, 2, 0) for orange at 2%

**Expected Results**:

- Blue animation: led_color_brightness(BLUE, 5) → (0, 0, 5)
- Orange animation: led_color_brightness(ORANGE, 5) → (5, 2, 0)
- Visually dim, not overwhelming position indicator

#### TST-DSP-ANIM-5.2: Position Brightness (100%)

**Test Steps**:

1. Set position in "too close" zone
2. Verify green position LED at full brightness (0, 255, 0)
3. Visually compare to animation LED
4. Confirm position LED clearly visible over animation

**Expected Results**:

- Position LED: GREEN (0, 255, 0) at 100% brightness
- Clearly distinguishable from 2% animation layer
- Visual dominance appropriate for user feedback

---

## Phase 4: Emergency Pattern Tests

### TST-DSP-ANIM-6: Emergency Blinking Pattern

**Covers**: REQ-DSP-ANIM-05, REQ-DSP-VISUAL-03, DSN-DSP-ANIM-03  
**Type**: System Test  
**Priority**: Mandatory  

#### TST-DSP-ANIM-6.1: Blink Frequency (1 Hz)

**Test Steps**:

1. Set distance below minimum
2. Measure time between blink state toggles
3. Verify toggle interval = 500ms ± 50ms
4. Measure over 10 blink cycles
5. Calculate average frequency

**Expected Results**:

- Blink period: 1000ms (500ms ON + 500ms OFF)
- Frequency: 1 Hz ± 0.1 Hz
- Consistent timing across multiple cycles

#### TST-DSP-ANIM-6.2: LED Pattern (Every 10th)

**Test Steps**:

1. Set distance below minimum
2. Set blink_on = true
3. Call render_frame()
4. For i = 0 to 39:
   - If i % 10 == 0: Verify LED i is RED
   - Else: Verify LED i is OFF

**Expected Results**:

- LEDs 0, 10, 20, 30 show red during ON phase
- All other LEDs off
- Pattern consistent during OFF phase (all LEDs off)

---

## Phase 5: Timing and Performance Tests

### TST-DSP-ANIM-7: Animation Timer Performance

**Covers**: REQ-DSP-ANIM-04 AC-6, DSN-DSP-ANIM-04  
**Type**: Performance Test  
**Priority**: Mandatory  

#### TST-DSP-ANIM-7.1: Frame Rate (10 FPS)

**Test Steps**:

1. Start animation timer
2. Count timer callbacks over 10 seconds
3. Verify callback count = 100 ± 2
4. Measure jitter between callbacks
5. Calculate average frame rate

**Expected Results**:

- Timer period: 100ms per callback
- Frame rate: 10 FPS ± 0.2 FPS
- Jitter: < 10ms between callbacks

#### TST-DSP-ANIM-7.2: Callback Execution Time

**Test Steps**:

1. Measure animation_timer_callback() execution time
2. Test under various conditions:
   - No animation active
   - "Too far" animation active
   - "Too close" animation active
   - Emergency blinking active
3. Record maximum execution time

**Expected Results**:

- Callback execution: < 1ms in all cases
- No blocking delays
- Timer task not starved
- Distance sensor task (priority 6) not affected

---

## Phase 6: Zone Transition Tests

### TST-DSP-ANIM-8: Smooth Zone Transitions

**Covers**: REQ-DSP-VISUAL-02, DSN-DSP-ALGO-01  
**Type**: System Test  
**Priority**: High  

#### TST-DSP-ANIM-8.1: Far to Ideal Transition

**Test Steps**:

1. Start with position = 20 (too far)
2. Verify blue animation running
3. Verify green position LED at 20
4. Move position to 13 (ideal zone boundary)
5. Verify animation stops
6. Verify all ideal zone LEDs (10-13) turn red
7. No glitches or flicker during transition

**Expected Results**:

- Animation stops immediately at ideal_end
- All ideal zone LEDs turn red simultaneously
- Position LED (green) disappears
- Transition smooth, no visible glitches

#### TST-DSP-ANIM-8.2: Close to Ideal Transition

**Test Steps**:

1. Start with position = 5 (too close)
2. Verify orange animation running
3. Verify green position LED at 5
4. Move position to 10 (ideal zone boundary)
5. Verify animation stops
6. Verify all ideal zone LEDs (10-13) turn red
7. No glitches or flicker during transition

**Expected Results**:

- Animation stops immediately at ideal_start
- All ideal zone LEDs turn red simultaneously
- Position LED (green) disappears
- Transition smooth, no visible glitches

#### TST-DSP-ANIM-8.3: Ideal to Emergency Transition

**Test Steps**:

1. Start in ideal zone (position = 11)
2. Verify all ideal zone LEDs red
3. Move distance below minimum
4. Verify blinking pattern activates
5. Verify LEDs 0, 10, 20, 30 blink red
6. Move back to ideal zone
7. Verify blinking stops, ideal zone red resumes

**Expected Results**:

- Emergency pattern overrides ideal zone immediately
- Ideal zone resumes when distance returns to range
- No stuck states or missed transitions

---

## Phase 7: Out of Range Tests

### TST-DSP-ANIM-9: Out of Range Behavior

**Covers**: REQ-DSP-VISUAL-04, DSN-DSP-ALGO-01  
**Type**: System Test  
**Priority**: High  

#### TST-DSP-ANIM-9.1: Above Maximum (Animation Only)

**Test Steps**:

1. Set distance > max_distance_mm
2. Verify no green position LED shown
3. Verify blue "too far" animation running
4. Verify animation provides directional cue only

**Expected Results**:

- No position indicator (measurement invalid)
- Blue animation visible (directional guidance)
- Animation runs from LED 39 toward ideal_end
- Clear distinction from valid range behavior

#### TST-DSP-ANIM-9.2: Sensor Timeout (All Off)

**Test Steps**:

1. Set sensor status = DISTANCE_SENSOR_TIMEOUT
2. Call update_led_display()
3. Verify all LEDs turn off
4. Verify animation stops

**Expected Results**:

- All LEDs off (no measurement available)
- Animation deactivated
- System ready to resume on valid measurement

---

## Manual Visual Validation

### TST-DSP-ANIM-10: Real-World Scenario Tests

**Type**: Visual Test  
**Priority**: High  

#### TST-DSP-ANIM-10.1: Parking Approach Simulation

**Test Scenario**: Simulate vehicle approaching parking space from far away

**Test Steps**:

1. Start with distance at maximum (e.g., 50cm)
2. Slowly decrease distance toward ideal zone
3. Observe:
   - Blue animation guiding toward ideal zone
   - Green position LED tracking distance
   - Smooth animation motion (10 FPS)
   - Clear visual distinction between layers
4. Enter ideal zone (around 12cm for 40 LEDs, 10-50cm range)
5. Observe all ideal zone LEDs turn red (STOP)
6. Continue moving closer
7. Observe ideal zone disappear, orange "too close" animation
8. Move below minimum
9. Observe emergency blinking pattern

**Expected Results**:

- Clear directional guidance from blue animation
- Position feedback from green LED
- Unmistakable STOP signal (all red) in ideal zone
- Warning guidance from orange animation when too close
- Emergency alert from blinking pattern when dangerously close
- Intuitive user experience throughout approach

#### TST-DSP-ANIM-10.2: Color Recognition Test

**Test Steps**:

1. Test with multiple users (if possible)
2. Ask users to identify:
   - "Which color means move closer?" (Blue)
   - "Which color means move away?" (Orange)
   - "Which color means STOP?" (Red)
   - "Which color shows current position?" (Green)
3. Verify color choices intuitive without training

**Expected Results**:

- Blue recognized as "move closer" directional cue
- Orange recognized as "move away" directional cue
- Red universally recognized as STOP
- Green clearly indicates current position
- No user confusion or ambiguity

---

## Test Automation

### GitHub Copilot Test Implementation

**Implementation Scope**: Automated test suite for zone calculation, animation state, and layer compositing

**Copilot Assignment**:

1. **Unit Test Generation**: Implement automated tests for calculate_zones(), update_animation_state(), advance_animation()
2. **Integration Test Scripts**: Create test scripts for render_frame() with various zone scenarios
3. **Performance Benchmarks**: Generate timing measurement code for animation_timer_callback()
4. **Simulation Harness**: Create test harness for simulating distance measurements and verifying LED output

**Human Validation**:

- Manual visual testing of colors, brightness, and patterns
- Real-world parking scenario validation
- User experience validation
- Hardware-specific timing verification (ESP32-specific)

---

## Success Criteria

All tests must pass before declaring dual-layer animation system complete:

- [x] Zone calculation correct for 20, 40, 60, 100 LED configurations
- [x] Animation state machine transitions properly between zones
- [x] Dual-layer rendering respects priority (Emergency > Ideal > Position > Animation)
- [x] Colors and brightness match specifications (2% animation, 100% position/ideal/emergency)
- [x] Emergency blinking at 1 Hz on every 10th LED
- [x] Animation frame rate stable at 10 FPS
- [x] Timer callback execution < 1ms
- [x] Zone transitions smooth and glitch-free
- [x] Out of range handling correct (animation only above max, all off on timeout)
- [x] Manual visual validation confirms intuitive user experience

---

*This document follows OpenFastTrack methodology for test traceability.*
