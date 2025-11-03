Known Issues and Limitations
=============================

This document tracks known issues, limitations, and areas requiring improvement in the ESP32 Template.

Component Initialization Order
-------------------------------

**Issue:** Component initialization sequence has implicit dependencies that are not formally documented.

**Current Behavior:**

The initialization order in ``main.c`` follows this sequence:

1. NVS Flash initialization
2. Configuration Manager
3. Web Server (includes WiFi Manager)
4. QEMU Network Tunnel (if emulator build)

**Problem:**

- Web server depends on NVS being initialized (for WiFi credentials)
- Configuration manager depends on NVS
- Network tunnel depends on lwIP stack being initialized
- Dependencies are implicit in code, not enforced by build system

**Impact:**

- Easy to break initialization order when modifying ``main.c``
- New developers may not understand dependencies
- Refactoring requires careful analysis

**Potential Solutions:**

1. **Explicit Dependency Graph**: Document component dependencies in architecture design
2. **CMake Dependency Checking**: Use ``idf_component_register(REQUIRES ...)`` more strictly
3. **Runtime Validation**: Add assert checks for prerequisite initialization
4. **Init Manager Pattern**: Create initialization manager that enforces order

**Status:** Open - needs architecture discussion

**Workaround:** Follow existing ``main.c`` initialization sequence carefully


HTTPS in QEMU Emulation
------------------------

**Issue:** HTTPS server does not work correctly in QEMU emulation.

**Current Behavior:**

- HTTP works fine in QEMU via UART tunnel
- HTTPS connections fail or hang
- Certificate handler component builds successfully

**Root Cause:** Not yet fully diagnosed. Potential issues:

- QEMU UART tunnel may not handle SSL/TLS traffic correctly
- Certificate validation issues in emulated environment
- mbedTLS configuration incompatibility

**Impact:**

- HTTPS testing requires real hardware
- Cannot fully test secure web interface in emulator

**Status:** Open - WIP

**Workaround:** Use HTTP for QEMU testing, test HTTPS on real ESP32 hardware


DNS Captive Portal
------------------

**Issue:** DNS server for captive portal detection does not work reliably.

**Current Behavior:**

- DNS server component exists but captive portal redirect is unreliable
- Some devices don't trigger captive portal popup
- Manual navigation to ``http://192.168.4.1`` required in AP mode

**Root Cause:**

- Platform-specific captive portal detection varies (iOS vs Android vs Windows)
- DNS redirect logic may not handle all query types correctly

**Impact:**

- Reduced user experience in AP mode configuration
- Users must manually discover device IP address

**Status:** Open - low priority

**Workaround:** Document explicit IP address access in user documentation


Flash Memory for HTTPS
-----------------------

**Issue:** 2MB flash modules may have insufficient space for full HTTPS implementation.

**Current Status:** ✅ Resolved

**Solution:** Template now configured for 4MB flash with Single App Large partition table, providing ~41% free space after base system.

**Recommendation:** Use ESP32 modules with 4MB flash (most common variant).


Build System Complexity
-----------------------

**Issue:** CMake build system with conditional QEMU/hardware component selection adds complexity.

**Trade-off:**

- ✅ Clean separation of simulator vs hardware code
- ✅ No ``#ifdef`` pollution in application code
- ❌ More complex build system
- ❌ Steeper learning curve for ESP-IDF beginners

**Status:** Accepted trade-off - benefits outweigh complexity

**Documentation:** See :doc:`qemu-emulator` and :doc:`qemu-network-internals` for details


Contributing
------------

If you encounter additional issues or have solutions to existing ones:

1. Check if issue already documented here
2. Report via GitHub Issues
3. Provide detailed reproduction steps
4. Include ESP-IDF version, hardware variant, build configuration

----

*Last Updated: October 2025*
