# GitHub Coding Agent Task: ESP32 Template Cleanup

## Task Overview

Transform the ESP32 distance sensor project into a clean, reusable template for GitHub Codespaces development with QEMU emulation support.

## Full Implementation Plan

See: `/workspaces/esp32-template/temp/CODING_AGENT_PLAN.md`

This document contains:

- 7 phases with detailed step-by-step instructions
- Exact commands to run
- Files to delete, rewrite, or modify
- Code examples for new implementations
- Testing procedures
- Success criteria

## Quick Summary

### Remove

- Components: `distance_sensor`, `led_controller`, `display_logic`, `startup_tests`
- Directory: `workshop/`
- Project-specific requirement and design docs
- File: `main/config.h`
- Local dev container setup references

### Keep & Simplify

- Components: `config_manager` (NVS example), `web_server` (generic + captive portal), `cert_handler`, `netif_uart_tunnel`
- Documentation: Requirements/design structure (make template)
- Legal docs (for GitHub Pages)
- All tools (QEMU, http_proxy, etc.)

### Rewrite

- `main/main.c` - Minimal Hello World
- `README.md` - Template introduction
- `docs/index.md` - Template overview
- `.github/copilot-instructions.md` - Generic ESP32 + template guidance

## Execution Order

1. Phase 1: Remove project-specific components and docs
2. **Test build** ← Important!
3. Phase 2: Simplify remaining components
4. **Test build** ← Important!
5. Phase 3: Rewrite core application
6. **Test build** ← Important!
7. Phase 4: Update documentation
8. **Test docs build**
9. Phase 5: Update GitHub configuration
10. Phase 6: Update MkDocs
11. Phase 7: Full testing suite

## Critical Requirements

- ✅ Work on `cleanup` branch
- ✅ Test build after phases 1, 2, 3
- ✅ Preserve QEMU network bridge functionality
- ✅ Keep HTTP proxy tool (needed for QEMU web access)
- ✅ Focus on Codespaces (remove local dev setup)
- ✅ Document HTTPS as known limitation
- ✅ Commit frequently with clear messages

## Success Criteria

- [ ] `idf.py build` succeeds
- [ ] QEMU runs with network bridge
- [ ] HTTP proxy forwards to localhost:8888
- [ ] Documentation builds without errors
- [ ] Pre-commit hooks pass
- [ ] No "distance sensor" references remain
- [ ] Template is ready for GitHub Pages
- [ ] README explains how to use template

## Known Issues

- **HTTPS not working** - Document as limitation, add TODO comments
- **HTTP proxy required** - Document as QEMU networking requirement

## Reference Documents

- `/workspaces/esp32-template/temp/CODING_AGENT_PLAN.md` ← **Main implementation guide**
- `/workspaces/esp32-template/temp/CLEANUP_PLAN.md` - Detailed analysis
- `/workspaces/esp32-template/temp/DISCUSSION_POINTS.md` - Decision rationale
- `/workspaces/esp32-template/temp/IMPLEMENTATION_SUMMARY.md` - Executive summary

---

**Please proceed with the implementation using CODING_AGENT_PLAN.md as your guide.**

