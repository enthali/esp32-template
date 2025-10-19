ESP32 Template Architecture Overview
======================================

This document provides a high-level architectural overview of the ESP32 Template project structure, focusing on generic patterns applicable to any ESP32 IoT application.

.. note::
   This is a **template architecture**. Users should adapt this to their specific application requirements by adding their own hardware components and business logic.

System Overview
---------------

The ESP32 Template is designed as a foundation for IoT devices with:

- **Hardware Platform**: ESP32 microcontroller (any variant with 4MB+ flash)
- **Network Connectivity**: WiFi (AP/STA modes) with automatic fallback
- **Configuration Interface**: Web-based configuration UI with captive portal
- **Development Environment**: Full QEMU emulation support for hardware-free development
- **Documentation**: Professional requirements engineering with Sphinx-Needs

Architecture Diagram
--------------------

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

Component Architecture
----------------------

Core Template Components
~~~~~~~~~~~~~~~~~~~~~~~~

Configuration Manager
^^^^^^^^^^^^^^^^^^^^^

**Purpose**: Demonstrates persistent configuration patterns with NVS storage.

**Location**: ``main/components/config_manager/``

**Key Features**:

- NVS-backed persistent storage
- Runtime configuration caching for fast access
- Parameter validation framework
- Factory reset capability
- Error recovery on corruption

**Usage Pattern**: Template for user-specific configuration needs.

Web Server
^^^^^^^^^^

**Purpose**: Example HTTP server with WiFi management and captive portal.

**Location**: ``main/components/web_server/``

**Key Features**:

- HTTP server with static file serving
- Captive portal for initial WiFi setup
- WiFi credential storage in NVS
- Automatic AP/STA mode switching
- DNS server for captive portal detection

**Status**: ✅ HTTP complete, 🚧 HTTPS in progress

Certificate Handler
^^^^^^^^^^^^^^^^^^^

**Purpose**: Automated SSL certificate management (work in progress).

**Location**: ``main/components/cert_handler/``

**Key Features**:

- Build-time certificate generation
- OpenSSL and Python fallback support
- ESP-IDF EMBED_FILES integration
- 25-year certificate validity

**Status**: 🚧 Implementation in progress (HTTPS not working in QEMU yet)

Network Tunnel
^^^^^^^^^^^^^^

**Purpose**: QEMU-specific network bridge for emulator support.

**Location**: ``main/components/netif_uart_tunnel/``

**Key Features**:

- UART-based network tunneling
- Python bridge script for host side
- Transparent TCP/IP pass-through
- QEMU-only compilation (conditional)

**Status**: ✅ Complete QEMU network support

Data Flow Patterns
------------------

Configuration Management Flow
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: text

   Boot:      NVS → config_load() → Runtime Cache → Application
   Runtime:   Application → config_get() → Runtime Cache (fast)
   Update:    Web UI → config_save() → Validation → NVS → Cache
   Factory:   config_factory_reset() → Defaults → NVS → Cache

Network & User Interface Flow
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: text

   WiFi:      Boot → Load Credentials → STA Attempt → AP Fallback
   Portal:    DNS Redirect → Captive Portal → WiFi Setup → STA Mode
   Web UI:    HTTP Request → Handler → JSON Response → Browser
   Config:    Web Form → REST API → Validation → NVS → Applied

QEMU Emulator Flow
~~~~~~~~~~~~~~~~~~

.. code-block:: text

   Network:   ESP32 lwIP Stack → UART1 → Python Bridge → Host TUN
   Web:       Browser → HTTP Proxy → ESP32 Web Server → Response
   Debug:     GDB → QEMU Monitor → ESP32 Process → VS Code UI

Design Principles
-----------------

Component-Based Architecture
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- **Modularity**: Each functional unit is a separate component with clean APIs
- **Reusability**: Components designed for use in multiple projects
- **Testability**: Component isolation enables unit testing
- **Maintainability**: Clear separation of concerns

Hardware Abstraction
~~~~~~~~~~~~~~~~~~~~

- **Simulator Components**: QEMU-compatible implementations with identical APIs
- **Conditional Compilation**: Build system selects implementation automatically
- **No Code Pollution**: Application logic remains clean without ``#ifdef`` directives

Memory Optimization
~~~~~~~~~~~~~~~~~~~

- **Flash Configuration**: 4MB flash target with 41% free space
- **Heap Management**: Careful use of FreeRTOS heap capabilities
- **Stack Sizing**: Optimized per-task stack allocation
- **Static vs Dynamic**: Prefer static allocation for predictable memory usage

Real-Time Operation
~~~~~~~~~~~~~~~~~~~

- **FreeRTOS Tasks**: Concurrent operation with priority-based scheduling
- **Queue-Based Communication**: Non-blocking inter-task data flow
- **Event-Driven Design**: Responsive to hardware and network events
- **Interrupt Handling**: Minimal ISR code with deferred processing

Configuration Persistence
~~~~~~~~~~~~~~~~~~~~~~~~~~

- **NVS Storage**: ESP-IDF Non-Volatile Storage for settings
- **Validation**: All configuration changes validated before persistence
- **Defaults**: Sensible defaults for all parameters
- **Factory Reset**: Recovery mechanism for corrupted configuration

Development-Friendly
~~~~~~~~~~~~~~~~~~~~

- **QEMU Support**: Full emulation enables hardware-free development
- **Fast Iteration**: No flashing required during QEMU development
- **CI/CD Ready**: Automated testing without physical hardware
- **VS Code Integration**: Debugging with GDB and VS Code

QEMU Emulation Architecture
----------------------------

Network Bridge Architecture
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The template includes a sophisticated network bridge for QEMU emulation:

.. code-block:: text

   ┌─────────────────────────────────────────────────────────┐
   │  Host System (Linux/macOS)                              │
   │  ┌──────────┐    ┌──────────┐    ┌───────────────┐     │
   │  │ Browser  │───▶│ tun0     │◀──▶│ Python Bridge │     │
   │  │ :8080    │    │ 192.168. │    │ (serial_tun)  │     │
   │  └──────────┘    │ 100.1/24 │    └───────────────┘     │
   │                  └──────────┘            │ TCP:5556    │
   └──────────────────────────────────────────┼─────────────┘
                                              │
   ┌──────────────────────────────────────────┼─────────────┐
   │  ESP32 QEMU Emulator                     ▼             │
   │  ┌──────────┐    ┌──────────┐    ┌──────────┐         │
   │  │ Web      │◀──▶│ lwIP     │◀──▶│ UART1    │         │
   │  │ Server   │    │ TCP/IP   │    │ Driver   │         │
   │  │ :80      │    │ 192.168. │    └──────────┘         │
   │  └──────────┘    │ 100.2/24 │                         │
   │                  └──────────┘                         │
   └───────────────────────────────────────────────────────┘

Key Components:

1. **UART Tunnel Driver** (``netif_uart_tunnel_sim.c``): ESP32-side network interface
2. **Python Bridge** (``tools/serial_tun_bridge.py``): Host-side TUN device management
3. **HTTP Proxy** (``tools/http_proxy.py``): Browser access helper (port 8080 → 80)
4. **Launch Script** (``tools/run-qemu-network.sh``): Automated setup and teardown

QEMU Development Benefits
~~~~~~~~~~~~~~~~~~~~~~~~~~

✅ **Fast Iteration**: Instant rebuild-test cycles without hardware flashing

✅ **CI/CD Ready**: Automated testing in GitHub Actions

✅ **Cross-Platform**: Develop on any system with QEMU support

✅ **Full Network Stack**: Real TCP/IP connectivity via UART tunnel

✅ **Debugging Support**: GDB integration with VS Code for breakpoints and inspection

✅ **Cost Effective**: No physical hardware required during initial development

Hardware Abstraction Strategy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The template uses CMake-based component selection for QEMU support:

**Identical APIs**: Hardware and simulator components share the same header files

**Build System Selection**: CMake automatically selects implementation based on target

**Clean Application Code**: No ``#ifdef CONFIG_TARGET_EMULATOR`` in main application

Example Pattern
^^^^^^^^^^^^^^^

.. code-block:: c

   // component_name.h (shared header)
   esp_err_t component_init(void);
   esp_err_t component_read(uint32_t* value);

   // component_name.c (hardware implementation)
   esp_err_t component_init(void) {
       // Real hardware initialization
   }

   // component_name_sim.c (QEMU implementation)
   esp_err_t component_init(void) {
       // Simulated initialization
   }

   // CMakeLists.txt selects appropriate source file

Threading Model
---------------

FreeRTOS Task Organization
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: text

   Core 0: Application Tasks (user-defined)
   ├── Main Task (Priority 1)
   │   └── Application initialization and coordination
   └── User Tasks (Priority varies)
       └── Application-specific processing

   Core 1: WiFi/Network Stack (ESP-IDF managed)
   ├── WiFi Management Task (Priority 2+)
   │   └── Connection handling, event processing
   ├── TCP/IP Stack Tasks (Priority varies)
   │   └── lwIP network processing
   └── HTTP Server Tasks (Priority varies)
       └── Request handling, file serving

Design Guidelines:

- **Core 0**: User application tasks
- **Core 1**: Reserve for WiFi/network stack
- **Priority Range**: 0-25 (higher = more important)
- **Stack Monitoring**: Use ``uxTaskGetStackHighWaterMark()`` for tuning

Memory Management Strategy
~~~~~~~~~~~~~~~~~~~~~~~~~~

**Static Allocation**: Component structures at compile time

**Dynamic Allocation**: Use ``heap_caps_malloc()`` for runtime needs

**DMA Buffers**: Use ``MALLOC_CAP_DMA`` for peripheral buffers

**IRAM Usage**: Use ``IRAM_ATTR`` only for time-critical interrupt handlers

**Monitoring**: Regular heap checks with ``esp_get_free_heap_size()``

User Customization Guide
-------------------------

Template Usage Workflow
~~~~~~~~~~~~~~~~~~~~~~~~

1. **Fork Template**: Create your own repository from this template
2. **Open in Codespaces**: Zero-setup development environment
3. **Customize main.c**: Add your application logic
4. **Add Components**: Create hardware-specific components as needed
5. **Test in QEMU**: Validate without physical hardware
6. **Flash Hardware**: Deploy to physical ESP32 when ready

Adding Hardware Components
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Example: Adding a temperature sensor component

.. code-block:: bash

   # Create component directory
   mkdir -p main/components/temperature_sensor

   # Add header file
   # main/components/temperature_sensor/temperature_sensor.h

   # Add implementation
   # main/components/temperature_sensor/temperature_sensor.c

   # Add CMakeLists.txt
   # main/components/temperature_sensor/CMakeLists.txt

   # Update main component dependencies
   # Edit main/CMakeLists.txt

Follow ESP-IDF component guidelines for proper integration.

Configuration Customization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Update ``main/components/config_manager/`` to match your application:

1. Modify ``system_config_t`` structure with your parameters
2. Update NVS storage layout
3. Add validation rules for your parameters
4. Update web interface configuration page

See :doc:`/12_design/config-manager` for detailed design.

Recommended Next Steps
----------------------

For New Users
~~~~~~~~~~~~~

1. **Read Requirements**: Review :doc:`/11_requirements/index` for system capabilities
2. **Study Examples**: Examine example components for patterns
3. **Start with QEMU**: Test in emulator before hardware
4. **Customize Gradually**: Add features incrementally
5. **Document Changes**: Update requirements and design docs

For Production Projects
~~~~~~~~~~~~~~~~~~~~~~~

1. **Replace Certificates**: Generate production SSL certificates
2. **Enable HTTPS**: Complete cert_handler integration
3. **Add Authentication**: Implement web interface security
4. **Optimize Memory**: Profile and tune for your specific needs
5. **Add Monitoring**: Implement health checks and diagnostics
6. **Write Tests**: Add unit and integration tests

Related Documentation
---------------------

- :doc:`/11_requirements/index` - System and component requirements
- :doc:`/12_design/system-architecture` - Detailed system design
- :doc:`/12_design/config-manager` - Configuration management design
- :doc:`/31_traceability/index` - Requirements traceability

----

*Last Updated: October 2025*
