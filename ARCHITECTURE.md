# ESP32 Distance Measurement - Technical Architecture

## Overview

This document describes the technical architecture and implementation details for the ESP32 Distance Measurement with LED Strip Display project.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        Main Application                      │
├─────────────────────────────────────────────────────────────┤
│  Display Logic Module                                       │
│  - Distance to LED mapping                                  │
│  - Animation control                                        │
│  - Visual feedback patterns                                 │
├─────────────────┬───────────────────┬───────────────────────┤
│  LED Controller │  Distance Sensor  │    Web Server         │
│  Module         │  Module           │    Module             │
│  - WS2812       │  - HC-SR04        │    - HTTP Server      │
│  - RMT Backend  │  - Timing Control │    - WebSocket        │
│  - Color Mgmt   │  - Filtering      │    - JSON API         │
├─────────────────┼───────────────────┼───────────────────────┤
│           ESP-IDF Hardware Abstraction Layer                │
│  RMT | GPIO | Timer | WiFi | HTTP Server | FreeRTOS        │
└─────────────────────────────────────────────────────────────┘
```

## Module Specifications

### 1. LED Controller Module

**Purpose**: Low-level hardware abstraction for WS2812 LED strip control with RAM buffer management.

**Hardware Interface**:
- Configurable GPIO pin for WS2812 data line (RMT backend)
- Configurable number of individually addressable LEDs
- 5V power supply for LED strip

**Key Features**:
- Individual pixel control (set/get/clear)
- RAM buffer maintains current LED state
- Manual update trigger for performance optimization
- Color space management (RGB)
- Configurable LED count during initialization
- Read capability for current LED states

**Current Status**: ✅ **COMPLETED** - Component implemented in `components/led_controller/`

### 2. Distance Sensor Module

**Purpose**: Accurate distance measurement using HC-SR04 ultrasonic sensor with interrupt-driven timing and dual-queue architecture.

**Hardware Interface**:
- GPIO14: Trigger pin
- GPIO13: Echo pin (Updated from GPIO15)
- Timing-critical pulse measurement with ESP32 timers

**Key Features**:
- Interrupt-driven measurement with precise timing control
- Dual-queue system for non-blocking API access
- Signal filtering and error detection (timeout, out-of-range, invalid readings)
- Temperature compensation support
- Queue overflow detection and statistics
- Configurable measurement rate (default: 1Hz)
- Background FreeRTOS task (Priority 6) for real-time operation

**Current Status**: ✅ **COMPLETED** - Component implemented in `components/distance_sensor/`

### 3. Display Logic Module

**Purpose**: Business logic for converting distance measurements to visual LED patterns.

**Key Features**:
- Distance-to-LED position mapping algorithms (10cm-50cm → LEDs 0-39)
- Multiple display modes (running LED, range indicator, error states)
- Smooth transitions between states with real-time updates
- Color coding for different distance ranges (green/blue normal, red errors)
- Animation state management with FreeRTOS task (Priority 3)
- Error visual indicators (sensor timeout, out-of-range conditions)

**Current Status**: 🔄 **IN PROGRESS** - GitHub Issue assigned to @github-copilot (see [copilot_issue_display_logic.md](copilot_issue_display_logic.md))

### 4. Web Server Module

**Purpose**: Remote monitoring and configuration interface.

**Key Features**:
- HTTP server for web interface
- WebSocket for real-time updates
- JSON API for data exchange
- Responsive web interface
- Configuration management

## Data Flow

```
HC-SR04 Sensor → Distance Sensor Module → Display Logic Module → LED Controller Module → WS2812 Strip
                                     ↓
                              Web Server Module → HTTP/WebSocket Client
```

## Threading Model

```
Main Task (Core 0)
├── Distance Measurement Task (High Priority)
│   └── Periodic sensor readings (100ms)
├── LED Update Task (Medium Priority) 
│   └── Visual updates (100ms)
└── Web Server Task (Low Priority)
    └── HTTP request handling

Core 1: WiFi/Network Stack (ESP-IDF default)
```

## Memory Management

- **LED Buffer**: Dynamic allocation based on configured LED count (3 bytes per LED)
- **Sensor History**: Circular buffer for averaging (configurable size)
- **Web Server**: Dynamic allocation for HTTP responses
- **Stack Sizes**: Optimized per task requirements

## Performance Requirements

- **LED Update Rate**: 10Hz minimum for smooth visual feedback
- **Sensor Sampling**: 10Hz for responsive distance tracking
- **Web Interface**: 1-2 second latency acceptable
- **Memory Usage**: < 50KB total application memory

## Configuration Management

All configurable parameters exposed through:
- Compile-time defines in header files
- Runtime configuration via web interface
- ESP-IDF menuconfig integration

---

## Implementation Notes

- **LED Controller**: Hardware abstraction documented in `led_controller.h`
- **Distance Sensor**: Timing-critical implementation documented in `distance_sensor.h`  
- **Display Logic**: Business logic patterns documented in `display_logic.h`
- **Web Server**: HTTP/WebSocket API documented in `web_server.h`

Each module provides comprehensive API documentation in its respective header file.
