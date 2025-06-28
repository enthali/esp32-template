# ESP32 Distance Project - Implementation Plan

## Project Overview
A project that uses an ESP32 to measure distance with an ultrasonic sensor and displays the measurement on a WS2812 LED strip with a web interface.

## Implementation Steps

### Step 1: LED Strip Animation ✅ **COMPLETED**
- ✅ Configure WS2812 LED strip with RMT backend
- ✅ Implement basic LED animations (running light effect)
- ✅ Test all 40 LEDs for proper functionality
- ✅ Verify power requirements and stability
- ✅ Create modular test architecture with background task
- ✅ Implement comprehensive color accuracy and brightness tests
- ✅ **Component Architecture**: Moved `led_controller` to `components/` directory
- ✅ **Refactoring**: Clean API design with proper ESP-IDF component structure

**Deliverables Completed:**
- `components/led_controller/` - Hardware abstraction component
- `main/test/` - Comprehensive test suite with background task support
- Full 40-LED strip validation and power requirements analysis

---

### Step 2: Ultrasonic Sensor Integration ✅ **COMPLETED**
- ✅ Connect HC-SR04 sensor to designated GPIO pins (Trigger: GPIO14, Echo: GPIO13)
- ✅ Implement distance measurement algorithm with interrupt-driven timing
- ✅ Output readings to serial console with comprehensive logging
- ✅ Calibrate and validate sensor accuracy with temperature compensation
- ✅ **Component Architecture**: Moved `distance_sensor` to `components/` directory
- ✅ **Dual-Queue System**: Implemented non-blocking measurement architecture
- ✅ **Error Handling**: Comprehensive timeout, range, and validity checks
- ✅ **Integration**: Live distance measurements working in main application

**Deliverables Completed:**
- `components/distance_sensor/` - Interrupt-driven sensor component with dual-queue architecture
- Real-time distance measurements (1Hz) with queue overflow protection
- Complete error handling for timeout, out-of-range, and invalid readings
- Hardware configuration section in `main.c` for centralized pin management

**Technical Achievements:**
- Background FreeRTOS task (Priority 6) for real-time sensor readings
- Non-blocking API with `distance_sensor_get_latest()` for consumer tasks
- Queue overflow detection and statistics for performance monitoring

---

### Step 3: Distance-to-LED Mapping 🔄 **IN PROGRESS** (Assigned to @github-copilot)
- 🔄 **GitHub Issue Created**: [Feature: Implement LED Distance Visualization with Display Logic Component](copilot_issue_display_logic.md)
- 🔄 **Copilot Assignment**: Working on `copilot/fix-3` branch
- ⏳ Create `main/display_logic.h/c` - Business logic component for distance visualization
- ⏳ Implement distance-to-LED mapping algorithm (10cm-50cm → LEDs 0-39)  
- ⏳ Add visual error indicators (red LEDs for out-of-range conditions)
- ⏳ Integrate with existing sensor readings via non-blocking queue API
- ⏳ Remove background test task from main application flow
- ⏳ Add one-time hardware test on startup for LED strip validation

**Planned Architecture:**
```
Priority 6: Distance Sensor Task  (real-time measurements - existing)
Priority 3: Display Logic Task    (NEW - LED visualization)
Priority 2: Test Task             (background only, not started by default)  
Priority 1: Main Task             (coordination only)
```

**Key Requirements:**
- Distance Range: 10cm to 50cm mapped linearly to 40 LEDs (LED 0 to LED 39)
- Visual Feedback: Green/blue for normal range, red for error states
- Error Handling: Sensor timeout → all LEDs off, out-of-range → red indicators
- Update Rate: 1Hz matching sensor measurement interval

---

### Step 4: Web Interface 📋 **PLANNED**

#### **Step 4.1: WiFi Setup with Captive Portal** 📋 **PLANNED**
- 📋 **Captive Portal**: Use ESP-IDF's built-in captive portal example as foundation
- 📋 **SoftAP Mode**: ESP32 creates its own WiFi network ("ESP32-Distance-Sensor")
- 📋 **WiFi Configuration Page**: Allow users to connect to their home WiFi
- 📋 **Network Switching**: Ability to switch between AP mode and Station mode
- 📋 **DNS Server**: Redirect all DNS queries to ESP32 IP for captive portal detection

**Based on ESP-IDF Captive Portal Example:**
```c
// Key components found via Playwright research:
#include "esp_http_server.h"
#include "dns_server.h"
#include "esp_wifi.h"

// SoftAP setup with captive portal
wifi_init_softap();                    // Create ESP32 WiFi network
dhcp_set_captiveportal_url();          // DHCP Option 114 for modern devices
start_dns_server(&config);             // Redirect all DNS to ESP32
```

**Implementation:**
- Create basic captive portal that shows WiFi setup page
- Scan for available networks and present selection list
- Store WiFi credentials in NVS flash
- Switch to Station mode when credentials provided
- Fall back to AP mode if connection fails

---

#### **Step 4.2: Basic Static Web Interface** 📋 **PLANNED**  
- 📋 **Static HTML/CSS**: Simple responsive web interface
- 📋 **Basic HTTP Server**: Serve static files from ESP32 flash memory
- 📋 **Core Pages**: 
  - `/` - Main dashboard with current distance display
  - `/settings` - Basic configuration page
  - `/about` - System information page
- 📋 **Mobile Responsive**: Touch-friendly interface for smartphones

**Integration Points:**
- Use existing `distance_sensor_get_latest()` API for current readings
- Display current distance value (refreshed on page reload)
- Basic system status (uptime, WiFi connection, sensor health)

---

#### **Step 4.3: JSON API Endpoints** 📋 **PLANNED**
- 📋 **RESTful API**: JSON endpoints for programmatic access
- 📋 **Core Endpoints**:
  - `GET /api/distance` - Current distance measurement
  - `GET /api/status` - System health and statistics  
  - `GET /api/config` - Current configuration
  - `POST /api/config` - Update settings
- 📋 **Error Handling**: Proper HTTP status codes and error messages

**API Design:**
```c
// Example JSON responses
GET /api/distance -> {"distance": 25.4, "status": "ok", "timestamp": 1234567890}
GET /api/status   -> {"uptime": 3600, "wifi": "connected", "tasks": "healthy"}
```

---

#### **Step 4.4: Real-time Data Streaming** 📋 **PLANNED**
- 📋 **Server-Sent Events (SSE)**: Real-time distance updates
- 📋 **Live Dashboard**: Auto-updating web interface without page refresh
- 📋 **LED Visualization**: Browser-based LED strip representation
- 📋 **WebSocket Support**: Bidirectional communication (optional upgrade)

**Real-time Features:**
```c
// SSE endpoint for live updates
GET /events -> text/event-stream
data: {"distance": 25.4, "led_states": [...], "timestamp": 1234567890}
```

**Dashboard Features:**
- Live distance number with units
- Virtual LED strip matching physical hardware
- Color-coded status indicators
- Real-time error notifications

---

#### **Step 4.5: Advanced Configuration Interface** 📋 **PLANNED**
- 📋 **Settings Management**: Web-based configuration for all system parameters
- 📋 **Distance Calibration**: Set min/max ranges for LED mapping  
- 📋 **LED Configuration**: Brightness, color schemes, animation modes
- 📋 **Sensor Settings**: Timeout values, temperature compensation
- 📋 **WiFi Management**: Change networks, view connection status
- 📋 **System Controls**: Restart, factory reset, firmware updates

**Configuration Categories:**
- **Distance Mapping**: 10-50cm range, LED assignment, error thresholds
- **Visual Settings**: LED brightness, color patterns, error indicators  
- **Sensor Tuning**: Measurement intervals, timeout handling, temperature
- **Network Settings**: WiFi credentials, static IP, hostname
- **System Maintenance**: Logs, diagnostics, updates

---

**Step 4 Deliverables:**
- `main/wifi_manager.h/c` - Captive portal and WiFi configuration
- `main/web_server.h/c` - HTTP server with static file serving
- `main/api_handlers.h/c` - JSON API endpoints  
- `main/sse_server.h/c` - Server-sent events for real-time updates
- `main/www/` - Static web files (HTML, CSS, JavaScript)
- Complete mobile-responsive web interface
- Real-time dashboard with LED strip visualization
- Comprehensive configuration management

---

## Architecture Evolution

The project has evolved from a simple proof-of-concept to a clean, modular architecture:

1. **Hardware Abstraction**: Clean separation between hardware drivers (`components/`) and business logic (`main/`)
2. **Real-time Architecture**: Proper FreeRTOS task priorities and non-blocking communication
3. **Extensibility**: Ready for web interface integration with existing measurement and display systems
4. **Maintainability**: Comprehensive documentation, clean APIs, and modular test architecture

## Development Methodology

- **GitHub Integration**: Issues and Pull Requests for feature tracking
- **Component-First Design**: Reusable, testable hardware abstractions
- **Real-time Considerations**: Proper task priorities and non-blocking designs
- **Documentation-Driven**: Architecture and API documentation maintained alongside code
