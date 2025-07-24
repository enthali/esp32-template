# ESP32 Distance Project - Current Development Roadmap

This document contains the immediate next steps for the ESP32 Distance Project. Once these are completed, we'll pick the next items from `Features-intended.md`.

---

## Currently Planned: Configuration Management System 🔧

**Status**: In Development  
**Branch**: `docs/oft-requirements-structure`  
**Priority**: High  
**AI Implementation Target**: GitHub Copilot demonstration  

### Phase 1: Requirements and Documentation ✅
- ✅ Create OpenFastTrack documentation structure
- ✅ Define system requirements (REQ-SYS-1 through REQ-SYS-8)
- ✅ Specify configuration requirements (REQ-CFG-1 through REQ-CFG-11)
- ✅ Design architecture and APIs (DSN-CFG-1 through DSN-CFG-6)
- ✅ Create test specifications (TST-CFG-1 through TST-CFG-9)

### Phase 2: Implementation (Next)
- 🔄 **REQ-CFG-1**: Magic number consolidation into `main/config.h`
- 🔄 **REQ-CFG-2**: Source code migration to use centralized constants
- 📋 **REQ-CFG-3**: Runtime configuration data structures
- 📋 **REQ-CFG-4**: NVS persistent storage implementation
- 📋 **REQ-CFG-5**: Configuration API development

### Phase 3: Web Interface (Future)
- 📋 **REQ-CFG-7**: Web settings page
- 📋 **REQ-CFG-8**: Real-time configuration preview
- 📋 **REQ-CFG-9**: Configuration backup/restore

**Documentation**: See [`docs/requirements/config-requirements.md`](docs/requirements/config-requirements.md) for complete requirements.

**Demo Objective**: Showcase AI-assisted embedded development within formal safety-critical processes.

---

> **Note:**
>
> - This marks the first feature implemented using formal OFT methodology
> - GitHub Copilot will be assigned specific requirement sections for implementation
> - All implementation will be validated against formal test specifications
