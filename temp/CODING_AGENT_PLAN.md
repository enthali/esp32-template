# ESP32 Template Cleanup - GitHub Coding Agent Implementation Plan

**Date:** 2025-10-18  
**Branch:** cleanup  
**Target:** Transform distance sensor project into reusable ESP32 template

## Executive Summary

Transform the ESP32 distance sensor project into a clean, production-ready template that supports:

- ✅ **GitHub Codespaces ONLY** (remove local dev container references)
- ✅ **QEMU Emulation** with network bridge and HTTP proxy
- ✅ **Web Server** with captive portal (simplified main page)
- ✅ **Configuration Management** example (NVS usage pattern)
- ✅ **Auto API Documentation** structure (prepare for future generation)
- ✅ **Requirements/Design Template** (OpenFastTrack structure)
- ✅ **GitHub Pages** with legal documentation
- ✅ **Minimal main.c** (Hello World style)

## Decisions Finalized

| Area | Decision | Details |
|------|----------|---------|
| **Development Environment** | Codespaces only | Remove all local dev container references |
| **Components** | Keep simplified versions | `config_manager`, `web_server` (simplified), `cert_handler`, `netif_uart_tunnel` |
| **Components to Remove** | Delete entirely | `distance_sensor`, `led_controller`, `display_logic`, `startup_tests` |
| **Workshop** | Remove entirely | Not needed for template |
| **Documentation** | Keep structure, make template | Requirements/design structure, basic tool docs |
| **Legal Docs** | Keep | Needed for GitHub Pages |
| **API Docs** | Keep structure | Prepare for auto-generation feature |
| **Main.c** | Minimal | Simple Hello World with comments |
| **MCP Config** | Keep | Useful for GitHub integration |
| **Tools** | Keep all | Especially http_proxy for QEMU access |
| **HTTPS** | Fix if possible | Known issue to address |

---

## Phase 1: Remove Project-Specific Components

### 1.1 Delete Component Directories

**Action:** Remove hardware-specific components

```bash
rm -rf main/components/distance_sensor/
rm -rf main/components/led_controller/
rm -rf main/components/display_logic/
rm -rf main/components/startup_tests/
```

**Files affected:**

- `main/components/distance_sensor/`
- `main/components/led_controller/`
- `main/components/display_logic/`
- `main/components/startup_tests/`

### 1.2 Delete Workshop Directory

**Action:** Remove all workshop materials

```bash
rm -rf workshop/
```

**Files affected:**

- `workshop/` (entire directory)

### 1.3 Clean Project-Specific Documentation

**Action:** Remove project-specific requirement and design files, keep structure

**Keep (as templates):**

- `docs/requirements/README.md` - Convert to template guide
- `docs/design/README.md` - Convert to template guide
- `docs/architecture/README.md` - Keep structure

**Remove:**

- `docs/requirements/distance-sensor-requirements.md`
- `docs/requirements/led-controller-requirements.md`
- `docs/requirements/display-requirements.md`
- `docs/requirements/startup-test-requirements.md`
- `docs/design/distance-sensor-design.md`
- `docs/design/led-controller-design.md`
- `docs/design/display-design.md`
- `docs/design/startup-test-design.md`
- `docs/planning/Features-intended.md`
- `docs/test/config-tests.md`
- `docs/test/display-animation-tests.md`

**Convert to Template:**

- `docs/requirements/system-requirements.md` → Generic system requirements template
- `docs/requirements/config-requirements.md` → Keep as example
- `docs/requirements/web-server-requirements.md` → Keep as example
- `docs/design/system-design.md` → Generic system design template
- `docs/design/config-design.md` → Keep as example

**Commands:**

```bash
# Remove project-specific files
rm -f docs/requirements/distance-sensor-requirements.md
rm -f docs/requirements/led-controller-requirements.md
rm -f docs/requirements/display-requirements.md
rm -f docs/requirements/startup-test-requirements.md
rm -f docs/design/distance-sensor-design.md
rm -f docs/design/led-controller-design.md
rm -f docs/design/display-design.md
rm -f docs/design/startup-test-design.md
rm -rf docs/planning/
rm -rf docs/test/
```

### 1.4 Clean Assets

**Action:** Remove project-specific images

```bash
# Review and remove distance sensor photos
# Keep generic diagrams if any
rm -f docs/assets/images/setup.jpg  # If exists
```

### 1.5 Remove Project-Specific Root Files

**Action:** Remove implementation summary

```bash
rm -f IMPLEMENTATION_SUMMARY.md
```

---

## Phase 2: Simplify Remaining Components

### 2.1 Simplify Web Server Component

**File:** `main/components/web_server/web_server.c` and related HTML files

**Actions:**

1. Simplify main page (`index.html` or embedded HTML) to generic template welcome
2. Keep captive portal functionality intact
3. Remove distance sensor specific endpoints and data
4. Keep configuration API endpoints as examples

**Changes needed:**

- Remove distance sensor data from web responses
- Simplify main page to "ESP32 Template - Welcome"
- Keep `/api/config` as example
- Keep captive portal redirect logic
- Update HTTP handlers to be generic

### 2.2 Simplify Config Manager

**File:** `main/components/config_manager/config_manager.c` and `.h`

**Actions:**

1. Review and keep only generic configuration parameters
2. Remove distance sensor specific configs
3. Keep WiFi settings, LED count as generic examples
4. Update comments to indicate template nature

**Keep as examples:**

- WiFi AP/STA configuration
- Generic integer/string parameter examples
- NVS storage/retrieval patterns

**Remove:**

- Distance sensor specific parameters
- LED controller specific parameters (or make generic)

### 2.3 Review Cert Handler

**File:** `main/components/cert_handler/`

**Actions:**

1. Keep if needed for HTTPS functionality
2. If HTTPS isn't working, keep but mark as "TODO: Fix HTTPS"
3. Add comments about HTTPS being a work-in-progress feature

**Decision:** Keep as-is with TODO comments for HTTPS fix

### 2.4 Keep Network Tunnel As-Is

**File:** `main/components/netif_uart_tunnel/`

**Action:** No changes needed - required for QEMU networking

---

## Phase 3: Rewrite Core Application

### 3.1 Minimal main.c

**File:** `main/main.c`

**New content structure:**

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Template Starting...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // TODO: Initialize your components here
    // Example: config_manager_init();
    // Example: wifi_manager_init();
    // Example: web_server_start();
    
    ESP_LOGI(TAG, "Template initialized. Add your application code here.");
    
    // Main loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "Template running...");
    }
}
```

**Action:** Rewrite `main/main.c` to minimal template

### 3.2 Simplify CMakeLists.txt

**File:** `main/CMakeLists.txt`

**Actions:**

1. Remove dependencies on deleted components
2. Keep dependencies on: `config_manager`, `web_server`, `cert_handler`, `netif_uart_tunnel`
3. Make them optional (commented examples)

**Update REQUIRES section:**

```cmake
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES 
        nvs_flash
        # Optional example components (uncomment as needed):
        # config_manager
        # web_server
        # cert_handler
)
```

### 3.3 Simplify Kconfig.projbuild

**File:** `main/Kconfig.projbuild`

**Keep only:**

```kconfig
menu "ESP32 Template Configuration"

    config TARGET_EMULATOR
        bool "Build for QEMU emulator"
        default n
        help
            Enable this option to build for QEMU emulator instead of real hardware.
            This will use simulator components that provide identical APIs but
            simulate hardware behavior without requiring physical devices.

    config EMULATOR_MOCK_SENSOR
        bool "Use mocked data in emulator"
        depends on TARGET_EMULATOR
        default y
        help
            Generate simulated data for testing. This provides visual
            feedback for testing without hardware.

endmenu
```

### 3.4 Remove config.h

**File:** `main/config.h`

**Action:** Delete entirely - was project-specific

```bash
rm -f main/config.h
```

### 3.5 Update Root CMakeLists.txt

**File:** `CMakeLists.txt`

**Action:** Change project name

```cmake
project(esp32-template)
```

---

## Phase 4: Update Documentation

### 4.1 Rewrite README.md

**File:** `README.md`

**New structure:**

```markdown
# ESP32 Template Project

A production-ready ESP32 project template with GitHub Codespaces, QEMU emulation, and modern development tools.

## Features

- 🚀 **GitHub Codespaces** - Zero-setup cloud development environment
- 🖥️ **QEMU Emulation** - Test without hardware, includes network support
- 🐛 **GDB Debugging** - Full debugging in QEMU
- 🌐 **Web Server** - HTTP server with captive portal example
- ⚙️ **Configuration Management** - NVS storage example
- 📝 **Documentation** - MkDocs with GitHub Pages
- 🤖 **GitHub Copilot** - AI-assisted development instructions
- ✅ **Quality Gates** - Pre-commit hooks

## Quick Start

1. Click "Use this template" to create your repository
2. Open in GitHub Codespaces
3. Build: `idf.py build`
4. Run in QEMU: Use VS Code task "Start QEMU Debug Server"
5. Access web interface: http://localhost:8888 (via QEMU bridge)

## Customizing This Template

[Guide for adapting the template]

## Project Structure

[Explanation of directory layout]

## Documentation

Full documentation: https://[your-username].github.io/[repo-name]/

## License

MIT License - see LICENSE file
```

### 4.2 Update docs/index.md

**File:** `docs/index.md`

**Action:** Rewrite as template overview, remove distance sensor specifics

### 4.3 Update Development Docs

**Files to update:**

- `docs/development/devcontainer.md` - Focus on Codespaces, remove local setup
- `docs/development/qemu-emulator.md` - Keep and enhance
- `docs/development/qemu-network-internals.md` - Keep as advanced guide
- `docs/development/debugging.md` - Keep GDB instructions

**Actions:**

- Remove Windows local dev container setup sections
- Emphasize Codespaces workflow
- Ensure QEMU and http_proxy usage is well documented

### 4.4 Convert Requirements/Design to Templates

**Files:**

- `docs/requirements/README.md` - Template guide for writing requirements
- `docs/design/README.md` - Template guide for design documents
- `docs/requirements/system-requirements.md` - Generic system template
- `docs/design/system-design.md` - Generic design template

**Action:** Add "TEMPLATE" markers and guide text

### 4.5 Update API Documentation Structure

**File:** `docs/api/README.md`

**Content:**

```markdown
# API Documentation

This directory will contain auto-generated API documentation.

## Future Enhancement

The template includes structure for automatic API documentation generation
from source code. This is a planned feature.

## Manual Documentation

Until auto-generation is implemented, document your APIs here:

- Endpoint specifications
- Request/response formats
- Error codes
- Examples
```

### 4.6 Keep Legal Documentation

**Files:** `docs/legal/impressum.md`, `docs/legal/datenschutz.md`

**Action:** Update with template placeholders

```markdown
# Impressum (Template)

**Replace this with your actual legal information**

[Your contact details]
[Legal requirements for your jurisdiction]
```

---

## Phase 5: Update GitHub Configuration

### 5.1 Update Copilot Instructions

**File:** `.github/copilot-instructions.md`

**Actions:**

1. Remove all distance sensor references
2. Update to "ESP32 Template Project"
3. Keep ESP32 coding standards
4. Add template-specific guidance:
   - How to use this template
   - QEMU and Codespaces workflow
   - Component architecture patterns
   - OpenFastTrack requirements/design approach

### 5.2 Update Prompt Snippets

**Directory:** `.github/prompt-snippets/`

**Actions:**

1. Review each file
2. Remove distance sensor specific references
3. Update to generic ESP32 guidance
4. Keep: `esp32-coding-standards.md`, `build-instructions.md`, `development.md`, `commit-message.md`
5. Update build instructions to remove Windows local setup

### 5.3 Review GitHub Workflows

**Files:** `.github/workflows/`

**Action:** No changes needed - deploy-docs and pre-commit are generic

### 5.4 Update Repository README

**File:** `.github/README.md`

**Action:** Update to reflect template nature

---

## Phase 6: Update MkDocs Configuration

### 6.1 Update mkdocs.yml

**File:** `mkdocs.yml`

**Actions:**

1. Update site_name to "ESP32 Template"
2. Update site_description
3. Review navigation structure
4. Remove distance sensor specific pages
5. Keep requirements/design/api structure

**Navigation structure:**

```yaml
nav:
  - Home: index.md
  - Getting Started:
      - Quick Start: development/README.md
      - GitHub Codespaces: development/devcontainer.md
      - QEMU Emulator: development/qemu-emulator.md
      - Debugging: development/debugging.md
  - Documentation Structure:
      - Requirements: requirements/README.md
      - Design: design/README.md
      - Architecture: architecture/README.md
      - API: api/README.md
  - Tools:
      - QEMU Network Bridge: development/qemu-network-internals.md
      - Pre-commit Hooks: development/pre-commit-hooks.md
  - Examples:
      - Config Management: requirements/config-requirements.md
      - Web Server: requirements/web-server-requirements.md
  - Legal:
      - Impressum: legal/impressum.md
      - Datenschutz: legal/datenschutz.md
```

---

## Phase 7: Testing and Validation

### 7.1 Build Testing

**Commands to run:**

```bash
# Clean build
idf.py fullclean
idf.py build

# Check for errors
idf.py size

# Build for emulator
idf.py -DCONFIG_TARGET_EMULATOR=y build
```

**Success criteria:**

- ✅ Project builds without errors
- ✅ No warnings about missing components
- ✅ Binary size is reasonable

### 7.2 QEMU Testing

**Commands to run:**

```bash
# Start QEMU with network bridge
./tools/run-qemu-network.sh

# In another terminal, check HTTP proxy
# Should be able to access web interface at localhost:8888
```

**Success criteria:**

- ✅ QEMU starts successfully
- ✅ Network bridge works
- ✅ HTTP proxy forwards requests
- ✅ Can access web interface (even if simplified)

### 7.3 Documentation Build

**Commands to run:**

```bash
mkdocs build --strict
mkdocs serve
```

**Success criteria:**

- ✅ Documentation builds without errors
- ✅ All links are valid
- ✅ Navigation structure makes sense

### 7.4 Pre-commit Hooks

**Commands to run:**

```bash
pre-commit run --all-files
```

**Success criteria:**

- ✅ Markdown linting passes
- ✅ No trailing whitespace
- ✅ File encoding correct

---

## Implementation Order

Execute in this order to minimize breakage:

1. **Phase 1** - Remove components and docs (less risky)
2. **Test build** - Ensure nothing else depends on removed components
3. **Phase 2** - Simplify remaining components
4. **Test build** - Ensure simplified components work
5. **Phase 3** - Rewrite core application
6. **Test build** - Ensure minimal app builds
7. **Phase 4** - Update documentation
8. **Test docs** - Build documentation
9. **Phase 5** - Update GitHub config
10. **Phase 6** - Update MkDocs
11. **Phase 7** - Full testing suite

---

## Known Issues to Address

### HTTPS Not Working

**Status:** Known issue  
**Action:** Add TODO comments in cert_handler  
**Future work:** Debug HTTPS in QEMU environment

**Files to update:**

- `main/components/cert_handler/cert_handler.c` - Add TODO comments
- `docs/known-limitations.md` - Document HTTPS limitation

### HTTP Proxy Required for QEMU Access

**Status:** Working as designed  
**Action:** Ensure well documented  
**Files:** `docs/development/qemu-network-internals.md`

---

## File Change Summary

### Files to Delete

```text
main/components/distance_sensor/
main/components/led_controller/
main/components/display_logic/
main/components/startup_tests/
workshop/
docs/requirements/distance-sensor-requirements.md
docs/requirements/led-controller-requirements.md
docs/requirements/display-requirements.md
docs/requirements/startup-test-requirements.md
docs/design/distance-sensor-design.md
docs/design/led-controller-design.md
docs/design/display-design.md
docs/design/startup-test-design.md
docs/planning/
docs/test/
main/config.h
IMPLEMENTATION_SUMMARY.md
```

### Files to Rewrite

```text
main/main.c - Minimal hello world
main/CMakeLists.txt - Simplified dependencies
main/Kconfig.projbuild - Only emulator flag
README.md - Template introduction
docs/index.md - Template overview
.github/copilot-instructions.md - Generic ESP32 + template guidance
```

### Files to Simplify/Update

```text
main/components/web_server/ - Remove distance sensor data
main/components/config_manager/ - Keep generic examples only
docs/requirements/README.md - Template guide
docs/design/README.md - Template guide
docs/requirements/system-requirements.md - Generic template
docs/design/system-design.md - Generic template
docs/development/devcontainer.md - Codespaces only
mkdocs.yml - Updated navigation
```

### Files to Keep As-Is

```text
main/components/netif_uart_tunnel/ - Required for QEMU
main/components/cert_handler/ - Keep for HTTPS future work
tools/ - All QEMU and network tools
.devcontainer/ - Codespaces configuration
.vscode/ - Editor configuration
.github/workflows/ - CI/CD pipelines
docs/legal/ - Update with template placeholders
docs/development/qemu-emulator.md - Essential documentation
docs/development/debugging.md - Essential documentation
```

---

## Success Criteria Checklist

- [ ] Project builds cleanly with `idf.py build`
- [ ] Project runs in QEMU with network bridge
- [ ] HTTP proxy allows web interface access at localhost:8888
- [ ] Documentation builds with `mkdocs build --strict`
- [ ] Pre-commit hooks pass on all files
- [ ] README clearly explains template usage
- [ ] No references to "distance sensor" in main code/docs
- [ ] All example components (config_manager, web_server) work
- [ ] GitHub Pages deploys successfully
- [ ] Template can be forked and customized easily
- [ ] Codespaces environment launches correctly
- [ ] QEMU debugging works with GDB

---

## Post-Cleanup Tasks

After successful cleanup:

1. Update GitHub repository description: "ESP32 project template with Codespaces and QEMU support"
2. Add repository topics: `esp32`, `template`, `codespaces`, `qemu`, `esp-idf`
3. Create release tag: `v1.0.0-template`
4. Update GitHub Pages deployment
5. Test template by creating new repository from it
6. Write blog post or announcement about template availability

---

## Notes for Coding Agent

- Work on branch `cleanup`
- Test build after each phase
- Commit frequently with descriptive messages
- If build breaks, investigate before proceeding
- Document any issues encountered
- Ask for clarification if requirements unclear
- Preserve git history - don't use force push

