System Architecture Design Specification
==========================================

This document defines the system-level architecture for the ESP32 Template, including component interactions, threading model, and data flow.

Document Metadata
-----------------

:Version: 1.0
:Date: 2025-07-24
:Author: ESP32 Template Team
:Requirements: REQ_SYS_1 through REQ_SYS_8


Overview
--------

.. spec:: ESP32 Component-Based Architecture
   :id: SPEC_SYS_ARCH_1
   :links: REQ_SYS_ARCH_1
   :status: approved
   :tags: architecture, system

   **Description:** The system follows ESP-IDF component-based architecture with clear separation of concerns.

   **System Structure:**

   .. code-block:: text

      ┌─────────────────────────────────────────────────────────────┐
      │                    Main Application (main.c)                │
      ├─────────────────────────────────────────────────────────────┤
      │                   Example Components                        │
      ├───────────────┬──────────────┬──────────────┬───────────────┤
      │ config_manager│  web_server  │ cert_handler │ netif_tunnel  │
      │  - NVS mgmt   │  - HTTP API  │  - SSL certs │  - QEMU net   │
      │  - Validation │  - WiFi mgr  │  - Auto-gen  │  - UART tun   │
      │  - Runtime    │  - Captive   │  - Embedding │  - Bridge     │
      ├───────────────┴──────────────┴──────────────┴───────────────┤
      │              ESP-IDF Hardware Abstraction Layer             │
      │   NVS | HTTP | WiFi | GPIO | FreeRTOS | Netif | TCP/IP     │
      └─────────────────────────────────────────────────────────────┘

   **Component Organization:**

   - **main/**: Application entry point and user-customizable code
   - **main/components/**: Reusable components following ESP-IDF conventions
   - **ESP-IDF HAL**: Hardware abstraction provided by ESP-IDF framework

   **Design Philosophy:** Template provides example components, users add their own application logic.


Component Specifications
-------------------------

.. spec:: Configuration Manager Component
   :id: SPEC_SYS_CONFIG_1
   :links: REQ_SYS_CFG_1
   :status: approved
   :tags: component, config

   **Description:** Example configuration management component demonstrating NVS usage patterns.

   **Component Purpose:**

   - Demonstrates persistent storage with ESP-IDF NVS
   - Shows runtime configuration structure patterns
   - Provides validation and error recovery examples
   - Template for user-specific configuration needs

   **Key Features:**

   - NVS-backed persistent storage
   - Runtime configuration caching
   - Parameter validation framework
   - Factory reset capability

   **Location**: ``main/components/config_manager/``

   **Status**: ✅ Complete example implementation


.. spec:: Web Server Component
   :id: SPEC_SYS_WEB_1
   :links: REQ_SYS_WEB_1
   :status: approved
   :tags: component, web

   **Description:** Example HTTP server component with captive portal and WiFi management.

   **Component Purpose:**

   - Demonstrates ESP-IDF HTTP server usage
   - Shows captive portal implementation pattern
   - Provides WiFi management examples (AP/STA modes)
   - Template for web-based device configuration

   **Key Features:**

   - HTTP server with static file serving
   - Captive portal for WiFi setup
   - WiFi credential storage in NVS
   - Automatic AP fallback on connection failure

   **Location**: ``main/components/web_server/``

   **Status**: ✅ Complete HTTP implementation, 🚧 HTTPS pending


.. spec:: Certificate Handler Component
   :id: SPEC_SYS_CERT_1
   :links: REQ_SYS_WEB_1
   :status: draft
   :tags: component, security

   **Description:** Certificate management component for HTTPS (work in progress).

   **Component Purpose:**

   - Automate SSL certificate generation
   - Embed certificates in firmware build
   - Enable HTTPS for web server component

   **Key Features:**

   - Build-time certificate generation
   - OpenSSL and Python fallback support
   - ESP-IDF EMBED_FILES integration
   - 25-year certificate validity

   **Location**: ``main/components/cert_handler/``

   **Status**: 🚧 **WIP** - HTTPS not yet working in QEMU


.. spec:: Network Tunnel Component
   :id: SPEC_SYS_NETIF_1
   :links: REQ_SYS_SIM_1
   :status: approved
   :tags: component, qemu, network

   **Description:** QEMU-specific network bridge component for emulator support.

   **Component Purpose:**

   - Enable network access in QEMU emulation
   - Bridge UART to host network stack
   - Support development without physical hardware

   **Key Features:**

   - UART-based network tunneling
   - Python bridge script for host side
   - Transparent TCP/IP pass-through
   - QEMU-only compilation (conditional)

   **Location**: ``main/components/netif_uart_tunnel/``

   **Status**: ✅ Complete QEMU network support


Threading and Task Model
-------------------------

.. spec:: FreeRTOS Task Architecture
   :id: SPEC_SYS_THREAD_1
   :links: REQ_SYS_ARCH_1
   :status: approved
   :tags: threading, rtos

   **Description:** Application uses FreeRTOS tasks for concurrent operations.

   **Task Structure:**

   .. code-block:: text

      Main Task (Priority 1, Core 0)
      └── Application-defined tasks (users add their own)

      WiFi/Network Tasks (Priority 2+, Core 1)
      ├── WiFi Management (connection handling)
      ├── HTTP Server (request processing)
      └── TCP/IP Stack (ESP-IDF managed)

      Core 1: WiFi/Network + ESP-IDF System Tasks (reserved)

   **Design Guidelines:**

   - Use Core 0 for application tasks
   - Core 1 reserved for WiFi/network stack
   - Prioritize tasks appropriately (0-25)
   - Monitor stack usage with uxTaskGetStackHighWaterMark()

   **Template Approach:** Users define their own tasks in main.c


.. spec:: Memory Management Strategy
   :id: SPEC_SYS_MEMORY_1
   :links: REQ_SYS_ARCH_1
   :status: approved
   :tags: memory, performance

   **Description:** Memory managed with FreeRTOS heap and ESP-IDF capabilities.

   **Memory Allocation:**

   - **Static**: Component structures allocated at compile time
   - **Dynamic**: Runtime allocations use heap_caps_malloc()
   - **DMA**: Use MALLOC_CAP_DMA for peripheral buffers
   - **IRAM**: Use IRAM_ATTR only for interrupt handlers

   **Memory Constraints:**

   - 4MB flash configured (typical ESP32 module)
   - ~41% free flash after base system
   - Monitor heap with esp_get_free_heap_size()
   - Stack overflow protection enabled

   **Template Guidance:** Users monitor memory usage for their specific application


Data Flow Design
----------------

.. spec:: Component Communication Pattern
   :id: SPEC_SYS_DATAFLOW_1
   :links: REQ_SYS_ARCH_1
   :status: approved
   :tags: dataflow, communication

   **Description:** Components communicate through well-defined APIs and FreeRTOS primitives.

   **Communication Patterns:**

   .. code-block:: text

      User Application (main.c)
           ↕ (API calls)
      Component Layer
           ↕ (FreeRTOS queues/events)
      ESP-IDF HAL
           ↕ (Hardware registers)
      Physical Hardware

   **Synchronization Primitives:**

   - **Mutexes**: Protect shared configuration state
   - **Queues**: Producer-consumer data flow
   - **Event Groups**: Task coordination and signaling
   - **Semaphores**: Resource counting and blocking

   **Template Pattern:** Components expose clean APIs; internal synchronization hidden


Network Architecture
--------------------

.. spec:: WiFi Manager Design
   :id: SPEC_SYS_WIFI_1
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


.. spec:: HTTP Server Architecture
   :id: SPEC_SYS_HTTP_1
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


QEMU Emulation Support
-----------------------

.. spec:: QEMU Emulation Architecture
   :id: SPEC_SYS_QEMU_1
   :links: REQ_SYS_SIM_1
   :status: approved
   :tags: qemu, emulation

   **Description:** Template supports QEMU emulation for hardware-less development.

   **QEMU Components:**

   1. **ESP32 QEMU Emulator**: Hardware emulation (CPU, peripherals)
   2. **UART Tunnel Driver**: Network bridge (netif_uart_tunnel)
   3. **Python Bridge Script**: Host-side network proxy
   4. **HTTP Proxy**: Web access from host browser

   **Network Architecture:**

   .. code-block:: text

      Host Browser → HTTP Proxy (tools/http_proxy.py)
                          ↓ (port 8080)
      QEMU ESP32 (192.168.5.2) → UART1 Tunnel
                          ↓ (serial_tun_bridge.py)
      Host Network Stack → Internet

   **Usage Workflow:**

   1. Start QEMU with network script: ``./tools/run-qemu-network.sh``
   2. ESP32 boots and initializes network tunnel
   3. Access web interface via HTTP proxy
   4. Test application without physical hardware

   **Limitations**: HTTPS not working in QEMU (HTTP works fine)

   **Status**: ✅ Complete QEMU support with network


Build and Configuration
-----------------------

.. spec:: ESP-IDF Build System Integration
   :id: SPEC_SYS_BUILD_1
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

   - ``sdkconfig``: ESP-IDF configuration (flash size, partition table)
   - ``CMakeLists.txt``: Build system definition
   - ``main/Kconfig.projbuild``: Custom menuconfig options

   **Build Commands:**

   .. code-block:: bash

      idf.py build              # Build firmware
      idf.py flash              # Flash to hardware
      idf.py monitor            # Serial monitor
      idf.py flash monitor      # Flash and monitor


.. spec:: Flash Memory Configuration
   :id: SPEC_SYS_FLASH_1
   :links: REQ_SYS_HW_1
   :status: approved
   :tags: flash, memory

   **Description:** Template configured for 4MB flash modules with single app partition.

   **Flash Configuration:**

   - **Flash Size**: 4MB (CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y)
   - **Partition Table**: Single App Large (maximum app space)
   - **Free Space**: ~41% available for application growth
   - **HTTPS Ready**: Sufficient space for SSL/TLS certificates

   **Partition Layout:**

   .. code-block:: text

      Name        Type  Offset   Size
      nvs         data  0x9000   24K   (configuration storage)
      phy_init    data  0xf000   4K    (RF calibration)
      factory     app   0x10000  ~3.8MB (application firmware)

   **Verification**: Use ``idf.py size`` to check memory usage


Performance Targets
-------------------

.. spec:: System Performance Requirements
   :id: SPEC_SYS_PERF_1
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
   - Check stack watermarks with uxTaskGetStackHighWaterMark()

   **Note**: Users optimize for their specific application requirements


Development Workflow
--------------------

.. spec:: GitHub Codespaces Development
   :id: SPEC_SYS_DEV_1
   :links: REQ_SYS_SIM_1
   :status: approved
   :tags: development, codespaces

   **Description:** Template optimized for GitHub Codespaces with pre-configured environment.

   **Development Environment:**

   - **DevContainer**: Ubuntu 24.04 with ESP-IDF 5.4.1 pre-installed
   - **QEMU**: Integrated for emulation without hardware
   - **VS Code**: Extensions for C/C++, Python, ESP-IDF
   - **Pre-commit Hooks**: Automated quality checks

   **Workflow:**

   1. Fork/clone template repository
   2. Open in GitHub Codespaces (or local devcontainer)
   3. Customize main.c and add components
   4. Build and test in QEMU
   5. Flash to physical hardware when ready

   **Benefits**: Zero local setup, consistent environment, works anywhere


Traceability
------------

**Requirements Coverage:**

This design specification implements the following system requirements:

- :need:`REQ_SYS_HW_1` - ESP32 hardware platform
- :need:`REQ_SYS_ARCH_1` - Component-based architecture
- :need:`REQ_SYS_PERF_1` - System performance
- :need:`REQ_SYS_CFG_1` - Configuration management
- :need:`REQ_SYS_WEB_1` - Web interface
- :need:`REQ_SYS_SIM_1` - QEMU emulation support

**Design Specifications:**

- SPEC_SYS_ARCH_1: Component-based architecture
- SPEC_SYS_CONFIG_1: Configuration manager component
- SPEC_SYS_WEB_1: Web server component
- SPEC_SYS_CERT_1: Certificate handler (WIP)
- SPEC_SYS_NETIF_1: Network tunnel component
- SPEC_SYS_THREAD_1: Threading model
- SPEC_SYS_MEMORY_1: Memory management
- SPEC_SYS_DATAFLOW_1: Component communication
- SPEC_SYS_WIFI_1: WiFi management
- SPEC_SYS_HTTP_1: HTTP server architecture
- SPEC_SYS_QEMU_1: QEMU emulation
- SPEC_SYS_BUILD_1: Build system
- SPEC_SYS_FLASH_1: Flash configuration
- SPEC_SYS_PERF_1: Performance targets
- SPEC_SYS_DEV_1: Development workflow
