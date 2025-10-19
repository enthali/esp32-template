High-Level Architecture Design Specification
=============================================

This document defines the high-level system architecture for the ESP32 Template, including component interactions, layering, and integration patterns.

Document Metadata
-----------------

:Version: 1.0
:Date: 2025-10-19
:Author: ESP32 Template Team
:Requirements: REQ_SYS_HW_1, REQ_SYS_ARCH_1, REQ_SYS_CFG_1, REQ_SYS_WEB_1, REQ_SYS_SIM_1

Overview
--------

.. spec:: ESP32 Template Layered Architecture
   :id: SPEC_ARCH_LAYERS_1
   :links: REQ_SYS_ARCH_1
   :status: approved
   :tags: architecture, layering

   **Description:** The system implements a four-layer architecture for clear separation of concerns.

   **Architecture Layers:**

   .. code-block:: text

      ┌─────────────────────────────────────────────────────────────┐
      │                   User Application Layer                    │
      │              (main.c - User customizable)                   │
      └─────────────────────────────────────────────────────────────┘
                                 ↕
      ┌─────────────────────────────────────────────────────────────┐
      │              Example Component Library                      │
      ├───────────────┬──────────────┬──────────────┬───────────────┤
      │ config_manager│  web_server  │ cert_handler │ netif_tunnel  │
      │  - NVS mgmt   │  - HTTP API  │  - SSL certs │  - QEMU net   │
      │  - Validation │  - WiFi mgr  │  - Auto-gen  │  - UART tun   │
      │  - Runtime    │  - Captive   │  - Embedding │  - Bridge     │
      └───────────────┴──────────────┴──────────────┴───────────────┘
                                 ↕
      ┌─────────────────────────────────────────────────────────────┐
      │          ESP-IDF Hardware Abstraction Layer                 │
      │   NVS | HTTP | WiFi | GPIO | FreeRTOS | Netif | TCP/IP     │
      └─────────────────────────────────────────────────────────────┘
                                 ↕
      ┌─────────────────────────────────────────────────────────────┐
      │            ESP32 Hardware / QEMU Emulator                   │
      └─────────────────────────────────────────────────────────────┘

   **Layer Responsibilities:**

   1. **User Application Layer**: Entry point (``main.c``), application-specific logic
   2. **Component Library**: Reusable example components (config, web, networking)
   3. **ESP-IDF HAL**: Framework-provided hardware abstraction
   4. **Hardware**: Physical ESP32 or QEMU emulation

   **Design Rationale:** Layered architecture enables:
   
   - Independent testing of each layer
   - Component reuse across projects
   - Clear upgrade paths when ESP-IDF updates
   - Hardware/emulator abstraction


Component Design Specifications
--------------------------------

.. spec:: Configuration Manager Component Design
   :id: SPEC_ARCH_CONFIG_1
   :links: REQ_SYS_CFG_1
   :status: approved
   :tags: component, config

   **Description:** Example component demonstrating NVS-based configuration patterns.

   **Purpose:** Provide reference implementation for persistent configuration management.

   **Key Design Decisions:**

   - **NVS Storage**: Uses ESP-IDF Non-Volatile Storage API
   - **Runtime Cache**: Configuration cached in RAM for fast access
   - **Validation Framework**: All parameters validated before persistence
   - **Factory Reset**: Recovery mechanism included

   **Location**: ``main/components/config_manager/``

   **API Pattern**: Getter/setter functions with validation

   **See Also**: :doc:`config-manager` for detailed design


.. spec:: Web Server Component Design
   :id: SPEC_ARCH_WEB_1
   :links: REQ_SYS_WEB_1
   :status: approved
   :tags: component, web, network

   **Description:** Example HTTP server with WiFi management and captive portal.

   **Purpose:** Provide reference implementation for web-based device configuration.

   **Key Design Decisions:**

   - **HTTP Server**: ESP-IDF ``esp_http_server`` component
   - **Static Files**: Embedded in firmware using ``EMBED_FILES``
   - **Captive Portal**: DNS server redirects all requests to device
   - **WiFi Manager**: Automatic AP/STA mode switching with NVS credentials
   - **Fallback Logic**: AP mode if STA connection fails

   **Location**: ``main/components/web_server/``

   **API Pattern**: Initialization function, REST endpoints for configuration

   **Status**: ✅ HTTP working, 🚧 HTTPS in progress

   **See Also**: Web server requirements in :doc:`../11_requirements/web-server`


.. spec:: Certificate Handler Component Design
   :id: SPEC_ARCH_CERT_1
   :links: REQ_SYS_WEB_1
   :status: draft
   :tags: component, security

   **Description:** Automated SSL certificate management for HTTPS (work in progress).

   **Purpose:** Enable HTTPS for web server without manual certificate management.

   **Key Design Decisions:**

   - **Build-Time Generation**: Certificates generated during build if missing
   - **Dual Tool Support**: OpenSSL binary (preferred) or Python cryptography fallback
   - **Firmware Embedding**: Uses ESP-IDF ``EMBED_FILES`` feature
   - **Long Validity**: 25-year certificate lifetime for device lifecycle

   **Location**: ``main/components/cert_handler/``

   **Status**: 🚧 Implementation in progress, HTTPS not working in QEMU yet

   **Known Limitation**: HTTPS support in QEMU requires additional testing


.. spec:: Network Tunnel Component Design
   :id: SPEC_ARCH_NETIF_1
   :links: REQ_SYS_SIM_1
   :status: approved
   :tags: component, qemu, network

   **Description:** QEMU-specific network bridge enabling full TCP/IP stack in emulation.

   **Purpose:** Enable hardware-free development with real network connectivity.

   **Key Design Decisions:**

   - **UART-Based Tunnel**: Uses UART1 for frame transport
   - **Ethernet Encapsulation**: IP packets wrapped in Ethernet frames
   - **Length-Prefix Protocol**: 2-byte big-endian length header per frame
   - **Python Bridge**: Host-side TUN device management
   - **Conditional Compilation**: Only built for QEMU target

   **Architecture:**

   .. code-block:: text

      ESP32 lwIP Stack → UART1 → Python Bridge → Host TUN Device

   **Location**: ``main/components/netif_uart_tunnel/``

   **Performance**: ~10 KB/s throughput (limited by 115200 baud UART)

   **See Also**: :doc:`../90_guides/qemu-network-internals` for implementation details


Data Flow Architecture
----------------------

.. spec:: Component Communication Pattern
   :id: SPEC_ARCH_COMM_1
   :links: REQ_SYS_ARCH_1
   :status: approved
   :tags: dataflow, communication

   **Description:** Components communicate through well-defined APIs and FreeRTOS primitives.

   **Communication Patterns:**

   .. code-block:: text

      User Application
           ↕ (Function calls)
      Component APIs
           ↕ (FreeRTOS primitives)
      ESP-IDF HAL
           ↕ (Hardware registers)
      Hardware

   **Synchronization Mechanisms:**

   - **Mutexes**: Protect shared configuration state
   - **Queues**: Producer-consumer data flow between tasks
   - **Event Groups**: Task coordination and signaling
   - **Semaphores**: Resource counting and blocking

   **Design Principle**: Components expose clean APIs; internal synchronization is hidden from users


.. spec:: Configuration Data Flow
   :id: SPEC_ARCH_CONFIG_FLOW_1
   :links: REQ_SYS_CFG_1
   :status: approved
   :tags: dataflow, config

   **Description:** Configuration flows through the system with caching and validation.

   **Flow Stages:**

   .. code-block:: text

      Boot:      NVS → config_load() → Runtime Cache → Application
      Runtime:   Application → config_get() → Runtime Cache (fast)
      Update:    Web UI → Validation → config_save() → NVS → Cache
      Factory:   Factory Reset → Defaults → NVS → Cache

   **Performance Optimization**: Runtime cache enables sub-microsecond config access

   **Data Integrity**: All updates validated before NVS write


.. spec:: Network Data Flow
   :id: SPEC_ARCH_NETWORK_FLOW_1
   :links: REQ_SYS_WEB_1
   :status: approved
   :tags: dataflow, network

   **Description:** Network traffic flows through WiFi manager and HTTP server.

   **WiFi Flow:**

   .. code-block:: text

      Boot → Load Credentials → STA Attempt → Success: Connected
                                            ↓ Failure (timeout)
                                           AP Mode → Captive Portal

   **HTTP Request Flow:**

   .. code-block:: text

      Browser → WiFi → lwIP Stack → HTTP Server → Handler → Response

   **DNS Redirect Flow** (Captive Portal):

   .. code-block:: text

      DNS Query → DNS Server → Redirect to Device IP (192.168.4.1)


.. spec:: WiFi Manager Design Details
   :id: SPEC_ARCH_WIFI_1
   :links: REQ_SYS_WEB_1
   :status: approved
   :tags: network, wifi

   **Description:** WiFi management with automatic reconnection and AP fallback.

   **WiFi Operation Modes:**

   1. **Station (STA) Mode**: Connect to existing WiFi network
   2. **Access Point (AP) Mode**: Create configuration network
   3. **Fallback Logic**: Auto-switch to AP if STA fails

   **Connection Flow:**

   .. code-block:: text

      Boot → Load NVS Credentials → STA Connection Attempt
                                           ↓ (success)
                                      STA Connected
                                           ↓ (failure after timeout)
                                      AP Mode + Captive Portal

   **Credential Management:**

   - WiFi SSID/password stored in NVS
   - Factory reset clears credentials
   - Web interface provides credential update

   **Status**: ✅ Complete with NVS integration


.. spec:: HTTP Server Architecture Details
   :id: SPEC_ARCH_HTTP_1
   :links: REQ_SYS_WEB_1
   :status: approved
   :tags: network, http, web

   **Description:** HTTP server provides web interface and REST API.

   **Server Features:**

   - Static file serving from embedded filesystem
   - RESTful API endpoints for configuration
   - Captive portal detection and redirect
   - CORS headers for development

   **URL Structure:**

   .. code-block:: text

      /                   → index.html (main page)
      /settings           → settings.html (configuration)
      /wifi-setup         → wifi-setup.html (captive portal)
      /api/config         → REST API (GET/POST)
      /api/wifi           → WiFi management API

   **Status**: ✅ HTTP working, 🚧 HTTPS in progress


QEMU Emulation Architecture
----------------------------

.. spec:: QEMU Hardware Abstraction
   :id: SPEC_ARCH_QEMU_1
   :links: REQ_SYS_SIM_1
   :status: approved
   :tags: qemu, emulation

   **Description:** Template supports QEMU emulation for hardware-free development.

   **Emulation Strategy:**

   - **Network Stack**: Full TCP/IP via UART tunnel (not WiFi simulation)
   - **Component Abstraction**: Optional simulator implementations with identical APIs
   - **Build System**: CMake automatically selects hardware vs. simulator components
   - **Clean Code**: No ``#ifdef`` conditionals in application code

   **Network Architecture:**

   .. code-block:: text

      Browser (Host) → HTTP Proxy → TUN Bridge → QEMU UART1 → ESP32 lwIP

   **Components:**

   1. **QEMU Emulator**: ESP32 hardware emulation
   2. **Network Tunnel**: ``netif_uart_tunnel_sim.c`` driver
   3. **TUN Bridge**: ``tools/serial_tun_bridge.py`` (Python)
   4. **HTTP Proxy**: ``tools/http_proxy.py`` for browser access

   **Benefits**:

   - Fast iteration without hardware flashing
   - CI/CD automation without physical devices
   - GDB debugging with VS Code integration
   - Cross-platform development


.. spec:: QEMU Component Selection
   :id: SPEC_ARCH_QEMU_BUILD_1
   :links: REQ_SYS_SIM_1
   :status: approved
   :tags: qemu, build

   **Description:** Build system automatically selects appropriate component implementations.

   **Selection Mechanism:**

   .. code-block:: cmake

      # Component CMakeLists.txt pattern
      if(CONFIG_TARGET_EMULATOR)
          set(COMPONENT_SRCS "component_sim.c")
      else()
          set(COMPONENT_SRCS "component.c")
      endif()

   **Configuration**: ``idf.py menuconfig`` → "Build for QEMU emulator"

   **Design Benefits:**

   - Same header files for both implementations
   - No code pollution with conditional compilation
   - Easy to add simulator support to any component


Threading Architecture
----------------------

.. spec:: FreeRTOS Task Organization
   :id: SPEC_ARCH_TASKS_1
   :links: REQ_SYS_ARCH_1
   :status: approved
   :tags: threading, rtos

   **Description:** Application uses FreeRTOS tasks with priority-based scheduling.

   **Task Structure:**

   .. code-block:: text

      Core 0: Application Tasks (user-defined)
      ├── Main Task (Priority 1)
      │   └── Initialization and coordination
      └── User Tasks (Priority varies)
          └── Application-specific logic

      Core 1: WiFi/Network Stack (ESP-IDF managed)
      ├── WiFi Management (Priority 2+)
      ├── TCP/IP Stack (lwIP)
      └── HTTP Server

   **Design Guidelines:**

   - Core 0 for application tasks
   - Core 1 reserved for WiFi/network (best performance)
   - Priority range: 0-25 (higher = more important)
   - Monitor stack with ``uxTaskGetStackHighWaterMark()``


.. spec:: Memory Management Strategy
   :id: SPEC_ARCH_MEMORY_1
   :links: REQ_SYS_HW_1
   :status: approved
   :tags: memory, performance

   **Description:** Memory managed with FreeRTOS heap and ESP-IDF capabilities.

   **Allocation Strategy:**

   - **Static**: Component structures at compile time (predictable)
   - **Dynamic**: Runtime allocations use ``heap_caps_malloc()``
   - **DMA Buffers**: Use ``MALLOC_CAP_DMA`` capability
   - **IRAM**: Use ``IRAM_ATTR`` only for time-critical ISRs

   **Memory Configuration:**

   - 4MB flash (CONFIG_ESPTOOLPY_FLASHSIZE_4MB)
   - ~41% free flash after base system
   - Monitor: ``esp_get_free_heap_size()``

   **Design Principle**: Prefer static allocation for predictable memory usage


Build System Integration
-------------------------

.. spec:: ESP-IDF CMake Integration
   :id: SPEC_ARCH_BUILD_1
   :links: REQ_SYS_HW_1
   :status: approved
   :tags: build, cmake

   **Description:** Project uses ESP-IDF CMake build system with component registration.

   **Build Structure:**

   .. code-block:: cmake

      # Top-level CMakeLists.txt
      cmake_minimum_required(VERSION 3.16)
      include($ENV{IDF_PATH}/tools/cmake/project.cmake)
      project(esp32-template)

      # Component CMakeLists.txt
      idf_component_register(
          SRCS "component.c"
          INCLUDE_DIRS "."
          REQUIRES esp_http_server nvs_flash
      )

   **Configuration Files:**

   - ``sdkconfig``: ESP-IDF configuration (flash, partition table)
   - ``CMakeLists.txt``: Build definitions
   - ``main/Kconfig.projbuild``: Custom menuconfig options


.. spec:: Flash Memory Configuration
   :id: SPEC_ARCH_FLASH_1
   :links: REQ_SYS_HW_1
   :status: approved
   :tags: flash, memory

   **Description:** Template configured for 4MB flash with optimized partitions.

   **Flash Layout:**

   - **Flash Size**: 4MB (suitable for most ESP32 modules)
   - **Partition Table**: Single App Large (maximizes app space)
   - **Free Space**: ~41% available for growth
   - **HTTPS Ready**: Sufficient space for SSL certificates

   **Partition Layout:**

   .. code-block:: text

      Name        Type  Offset   Size
      nvs         data  0x9000   24K   (config storage)
      phy_init    data  0xf000   4K    (RF calibration)
      factory     app   0x10000  ~3.8MB (firmware)

   **Verification**: ``idf.py size`` shows memory usage


Development Workflow Design
----------------------------

.. spec:: GitHub Codespaces Integration
   :id: SPEC_ARCH_CODESPACES_1
   :links: REQ_SYS_SIM_1
   :status: approved
   :tags: development, devcontainer

   **Description:** Template optimized for zero-setup development in GitHub Codespaces.

   **Development Environment:**

   - **DevContainer**: Ubuntu 24.04 with ESP-IDF v5.4.1 pre-installed
   - **QEMU**: Integrated emulator for hardware-free testing
   - **VS Code**: Pre-configured extensions (ESP-IDF, C/C++, Python)
   - **Pre-commit Hooks**: Quality gates for documentation

   **Workflow:**

   .. code-block:: text

      Fork Template → Open in Codespaces → Customize main.c →
      Build → Test in QEMU → Flash to Hardware

   **Benefits**: Consistent environment, no local setup, works in browser


Performance Targets
-------------------

.. spec:: System Performance Requirements
   :id: SPEC_ARCH_PERF_1
   :links: REQ_SYS_PERF_1
   :status: approved
   :tags: performance, requirements

   **Description:** Template targets reasonable performance for IoT applications.

   **Performance Targets:**

   - **Boot Time**: < 3 seconds to WiFi connection
   - **Web Response**: < 500ms for configuration API calls
   - **Memory Usage**: < 100KB application heap usage
   - **Task Latency**: < 100ms for application tasks

   **Monitoring:**

   - Use ESP_LOGI() for timing measurements
   - Monitor heap with esp_get_free_heap_size()
   - Check stack usage with uxTaskGetStackHighWaterMark()
   - Profile with ESP-IDF performance tools


Traceability
------------

**Requirements Coverage:**

This high-level architecture design implements the following system requirements:

- :need:`REQ_SYS_HW_1` - ESP32 hardware platform with 4MB flash
- :need:`REQ_SYS_ARCH_1` - Component-based architecture with ESP-IDF
- :need:`REQ_SYS_CFG_1` - Configuration management system
- :need:`REQ_SYS_WEB_1` - Web interface for device management
- :need:`REQ_SYS_SIM_1` - QEMU emulation for hardware-free development

**Design Specifications:**

- SPEC_ARCH_LAYERS_1: Layered architecture
- SPEC_ARCH_CONFIG_1: Configuration manager design
- SPEC_ARCH_WEB_1: Web server design
- SPEC_ARCH_CERT_1: Certificate handler design (WIP)
- SPEC_ARCH_NETIF_1: Network tunnel design
- SPEC_ARCH_COMM_1: Communication patterns
- SPEC_ARCH_CONFIG_FLOW_1: Configuration data flow
- SPEC_ARCH_NETWORK_FLOW_1: Network data flow
- SPEC_ARCH_WIFI_1: WiFi manager details
- SPEC_ARCH_HTTP_1: HTTP server details
- SPEC_ARCH_QEMU_1: QEMU abstraction
- SPEC_ARCH_QEMU_BUILD_1: Build system integration
- SPEC_ARCH_TASKS_1: Task organization
- SPEC_ARCH_MEMORY_1: Memory management
- SPEC_ARCH_BUILD_1: CMake integration
- SPEC_ARCH_FLASH_1: Flash configuration
- SPEC_ARCH_CODESPACES_1: Development workflow
- SPEC_ARCH_PERF_1: Performance targets

**Related Documentation:**

- Detailed component designs: :doc:`config-manager`
- Development guides: :doc:`../90_guides/index`
- Requirements: :doc:`../11_requirements/index`

----

*Last Updated: October 2025*
