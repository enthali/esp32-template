# ESP32 Distance Project - Feature Intentions

This document contains **unnumbered** feature intentions that may or may not be implemented. These are flexible ideas that can be reordered, modified, or moved to the planned roadmap as priorities become clear.

---

## HTTPS Security Implementation 💭 **on Hold**

- 💭 **HTTPS Server**: Replace HTTP with encrypted HTTPS using ESP32 SSL/TLS support
- 💭 **Self-Signed Certificates**: Generate and embed certificates for local IoT device use
- 💭 **Certificate Generation**: Build-time certificate creation and embedding
- 💭 **Mixed Mode Support**: HTTPS for production, HTTP fallback for development
- 💭 **Browser Compatibility**: Handle self-signed certificate warnings appropriately


### OTA Firmware Updates 💭 **INTENDED**

- **Over-The-Air Updates**: ESP32 OTA partition scheme and update mechanism
- **Version Management**: Firmware versioning and rollback capability
- **Update Server**: Simple HTTP/HTTPS server for firmware distribution
- **Security**: Signed firmware updates and secure boot
- **User Interface**: Web-based firmware update with progress indication

### Security Hardening 💭 **INTENDED**

- **WiFi Security**: WPA3 support and strong encryption
- **Web Interface Security**: HTTPS, session management, CSRF protection
- **Access Control**: Basic authentication for configuration pages
- **Network Security**: Firewall rules and secure communication
- **Credential Protection**: Encrypted storage of sensitive data

## Planned Requirement & Design Documents (to create)

The following requirement and design documents are planned and should be created to preserve traceability and guide implementation. Each entry includes a short rationale and the suggested document path.

- **Distance Sensor (requirements)**: `docs/requirements/distance-sensor-requirements.md` — functional specs, timing constraints, GPIO assignments, error handling, and requirement IDs (REQ-SNS-1..).
- **Distance Sensor (design)**: `docs/design/distance-sensor-design.md` — hardware interface details, ISR/timing strategy, FreeRTOS task layout, and memory considerations (DSN-SNS-1..).

- **LED Controller (requirements)**: `docs/requirements/led-controller-requirements.md` — visual modes, LED counts, update rates, power limits, and API surface (REQ-LED-1..).
- **LED Controller (design)**: `docs/design/led-controller-design.md` — WS2812 timing, buffer management, DMA/IRAM usage, and scheduling (DSN-LED-1..).

- **Web Server & Certificate Handler (requirements)**: `docs/requirements/web-server-requirements.md` — endpoints, captive portal UX, HTTPS/cert lifecycle, and security requirements (REQ-WEB-1..).
- **Web Server & Certificate Handler (design)**: `docs/design/web-server-design.md` — server architecture, build-time cert generation/embedding, HTTP->HTTPS redirect logic, and TLS buffer tuning (DSN-WEB-1..).

- **Startup Tests (requirements)**: `docs/requirements/startup-tests-requirements.md` — boot validation test cases, peripheral init checks, pass/fail criteria (REQ-TST-STARTUP-1..).
- **Startup Tests (design/test plan)**: `docs/test/startup-tests-design.md` — automated/manual test steps, expected log output, hardware setup, and scripts to run on host or device (TST-STARTUP-1..).

- **Other candidates**: review `wifi_manager`, `dns_server` and `config` modules for missing requirement/design docs and add as needed.

Each requirements document should use the `REQ-<AREA>-<NUMBER>` ID format and reference corresponding design documents for traceability.
