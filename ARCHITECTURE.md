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
- GPIO13: Echo pin  
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

**Current Status**: ✅ **COMPLETED** - Component implemented in `main/display_logic.h/c`

### 4. Web Server Module

**Purpose**: Remote monitoring and configuration interface with WiFi management.

**Key Features**:

- **Smart WiFi Management**: Auto-connect to stored credentials with AP fallback
- **Captive Portal**: Automatic configuration page with network scanning
- **HTTP Server**: Mobile-responsive web interface for status and settings
- **Credential Storage**: Secure WiFi credential management in NVS flash
- **Network Switching**: Seamless AP ↔ STA mode transitions
- **DNS Server**: Captive portal detection and auto-redirect
- **Reset Functionality**: Clear stored credentials and restart system

**Current Status**: ✅ **COMPLETED** (Step 4.1) - Basic implementation in `main/wifi_manager.h/c` and `main/web_server.h/c`

## Data Flow

```text
HC-SR04 Sensor → Distance Sensor Module → Display Logic Module → LED Controller Module → WS2812 Strip
                                     ↓
                              Web Server Module → HTTP Client (WiFi/Captive Portal)
```

## Threading Model

```text
Main Task (Priority 1, Core 0)
├── Distance Sensor Task (Priority 6, Core 0)
│   └── Real-time measurements (100ms intervals)
├── Display Logic Task (Priority 3, Core 0) 
│   └── Event-driven LED visualization (blocking sensor API)
└── WiFi/Web Server Tasks (Priority 2, Core 1)
    ├── WiFi Management (connection/AP switching)
    ├── HTTP Server (request handling)
    └── DNS Server (captive portal)

Core 1: WiFi/Network Stack + ESP-IDF System Tasks
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
