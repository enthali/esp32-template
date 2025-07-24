# ESP32 Distance Project - Implementation Plan

This document contains **concrete implementation tasks** ready for development. Tasks can be implemented either:
- 🤖 **Agent Mode**: Using GitHub Copilot in VS Code via this chat interface
- 🌐 **Web Mode**: Assigned to GitHub Copilot directly in the web interface
- 👨‍💻 **Manual**: Human implementation with AI assistance

---

## 🎯 Current Sprint: Configuration Management System

**Sprint Goal**: Implement Phase 1 of configuration management (REQ-CFG-1, REQ-CFG-2)  
**Branch**: `docs/oft-requirements-structure`  
**Status**: Ready for Implementation  
**Target**: AI-assisted development demonstration  

### Ready for Implementation 🚀

#### TASK-CFG-001: Create Centralized Configuration Header
**Assignment**: 🤖 **GitHub Copilot (Agent Mode)**  
**Requirements**: REQ-CFG-1  
**Acceptance Criteria**: 
- Create `main/config.h` with all identified magic numbers
- Include documentation for each constant (purpose, valid range)
- Follow ESP32 naming conventions
- Categories: Distance Sensor, LED Controller, WiFi, Web Server, System Timing

**Implementation Notes**:
```c
// Expected structure in main/config.h
#ifndef CONFIG_H
#define CONFIG_H

// === DISTANCE SENSOR CONFIGURATION ===
#define DEFAULT_DISTANCE_MIN_CM         10.0f      // Range: 5.0-100.0
#define DEFAULT_DISTANCE_MAX_CM         50.0f      // Range: 20.0-400.0
// ... (continue with all identified magic numbers)

#endif // CONFIG_H
```

#### TASK-CFG-002: Migrate Source Code References
**Assignment**: 🤖 **GitHub Copilot (Agent Mode)**  
**Requirements**: REQ-CFG-2  
**Dependencies**: TASK-CFG-001  
**Acceptance Criteria**:
- Update `main/main.c` to use config.h constants
- Update `main/wifi_manager.c` to use config.h constants
- Update `main/web_server.c` to use config.h constants
- Update `components/distance_sensor/` to use config.h constants
- Update `components/led_controller/` to use config.h constants
- Verify build succeeds with no magic numbers remaining

**Files to Update**:
- `main/main.c`: LED_COUNT, DISTANCE_INTERVAL, etc.
- `main/wifi_manager.c`: WIFI_AP_CHANNEL, WIFI_STA_MAXIMUM_RETRY, etc.
- `main/web_server.c`: Buffer sizes, timeouts, port numbers
- `components/distance_sensor/distance_sensor.c`: Queue sizes, timing constants
- `components/led_controller/led_controller.c`: WS2812 timing, color constants

### Backlog (Future Sprints) 📋

#### TASK-CFG-003: Runtime Configuration Data Structures
**Assignment**: 🌐 **GitHub Copilot (Web Mode)**  
**Requirements**: REQ-CFG-3  
**Dependencies**: TASK-CFG-002  
**Description**: Implement system_config_t structure with validation ranges

#### TASK-CFG-004: NVS Persistent Storage
**Assignment**: 🌐 **GitHub Copilot (Web Mode)**  
**Requirements**: REQ-CFG-4  
**Dependencies**: TASK-CFG-003  
**Description**: Implement NVS storage with corruption detection and recovery

#### TASK-CFG-005: Configuration API Implementation
**Assignment**: 🤖 **GitHub Copilot (Agent Mode)**  
**Requirements**: REQ-CFG-5, REQ-CFG-6  
**Dependencies**: TASK-CFG-004  
**Description**: Thread-safe configuration API with validation

---

## 🔄 Sprint Workflow

### Agent Mode Implementation
1. **Select Task**: Choose ready task from "Ready for Implementation"
2. **Load Context**: Reference formal requirements in `docs/requirements/`
3. **Implement**: Use GitHub Copilot in VS Code via chat interface
4. **Validate**: Check against acceptance criteria
5. **Test**: Run builds and basic functionality tests
6. **Update Status**: Mark task complete and move to next

### Web Mode Implementation
1. **Assign Task**: Copy task description to GitHub Copilot web interface
2. **Provide Context**: Include relevant requirement links and code context
3. **Review Output**: Validate generated code against acceptance criteria
4. **Integrate**: Apply changes and test
5. **Update Status**: Mark complete and assign next task

### Task Status Tracking
- 🚀 **Ready**: All dependencies met, clear acceptance criteria
- 🔄 **In Progress**: Currently being implemented
- ✅ **Complete**: Acceptance criteria met and tested
- 🔍 **Review**: Needs validation against requirements
- 🚫 **Blocked**: Waiting for dependencies or clarification

---

## 📋 Next Sprint Planning

After completing Configuration Management Phase 1:

### Sprint 2 Options:
1. **Configuration Management Phase 2**: Runtime configuration (TASK-CFG-003 through TASK-CFG-005)
2. **Shared Data Architecture**: Thread-safe sensor→web data sharing
3. **Real-time Data Streaming**: Server-Sent Events implementation
4. **JSON API Endpoints**: RESTful API for programmatic access

**Selection Criteria**: 
- AI implementation demonstration value
- Technical complexity suitable for AI assistance
- Building toward complete working system

---

> **🎯 Demo Success Metrics**:
> - AI successfully implements against formal requirements
> - Generated code passes validation tests
> - Traceability maintained from REQ → IMPL → TEST
> - Process demonstrates safety-critical development compatibility
