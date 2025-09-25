# ESP32 Distance Sensor - Documentation

This directory contains the formal documentation following OpenFastTrack (OFT) methodology for requirements engineering and traceability.

## Documentation Structure

```text
docs/
├── README.md                           # This file - documentation overview
├── .oft.yml                            # OpenFastTrack configuration
├── requirements/                       # Requirements specifications (OFT format)
│   ├── system-requirements.md          # High-level system requirements (REQ-SYS-*)
│   ├── distance-sensor-requirements.md # HC-SR04 sensor requirements (REQ-SNS-*)
│   ├── led-controller-requirements.md  # WS2812 LED strip requirements (REQ-LED-*)
│   ├── display-requirements.md         # Distance display logic (REQ-DSP-*)
│   ├── config-requirements.md          # Configuration management (REQ-CFG-*)
│   ├── startup-test-requirements.md    # Boot sequence testing (REQ-STARTUP-*)
│   └── web-server-requirements.md      # Web interface (REQ-WEB-*)
├── design/                            # Design specifications with bidirectional traceability
│   ├── system-design.md               # Overall system architecture (DSN-SYS-*)
│   ├── distance-sensor-design.md      # Hardware interface design (DSN-SNS-*)
│   ├── led-controller-design.md       # WS2812 timing and control (DSN-LED-*)
│   ├── display-design.md              # Distance-to-LED mapping (DSN-DSP-*)
│   ├── config-design.md               # Configuration system (DSN-CFG-*)
│   └── startup-test-design.md         # Boot test implementation (DSN-STARTUP-*)
└── test/                              # Test specifications and results
    └── config-tests.md                # Configuration system test cases
```

## OpenFastTrack (OFT) Methodology

This project demonstrates **requirements-driven embedded development** with complete bidirectional traceability:

### Documentation Architecture

**Requirements Documents** (7 total):
- **Standardized format**: Each requirement has unique ID, description, rationale, acceptance criteria, and verification method
- **Traceability matrices**: Link requirements to design elements with priority levels
- **Modular approach**: Component-specific requirements (sensor, LED, display, config, web, startup test)

**Design Documents** (6 total):
- **Implementation-focused**: Design elements map directly to code architecture
- **Reverse traceability**: Each design element references implementing requirements
- **ESP32-specific**: Hardware constraints, FreeRTOS patterns, ESP-IDF best practices

### Traceability Examples

```
REQ-SNS-1 (Distance Measurement) 
    ↓ implements
DSN-SNS-1 (HC-SR04 Timing Implementation)
    ↓ implements  
components/distance_sensor/distance_sensor.c (measure_distance function)
```

### Pragmatic Documentation Decisions

- **Web Server**: Requirements-only approach (no design doc) since implementation primarily integrates existing ESP-IDF components
- **System Level**: High-level requirements with detailed component breakdowns

## Target Audience

This project serves as a **demonstration** for:

- **Embedded Systems Engineers**: Modern ESP32/IoT development with formal documentation
- **Requirements Engineers**: OpenFastTrack methodology applied to hardware projects  
- **AI-Assisted Development**: Using GitHub Copilot with requirements-driven development

## Getting Started

### Quick Navigation

1. **System Overview**: Start with `requirements/system-requirements.md` for high-level understanding
2. **Component Details**: Review individual component requirements (sensor, LED, display, config)
3. **Implementation**: Follow design documents for technical architecture
4. **Development Planning**: See `planning/Features-intended.md` for documentation strategy

## Documentation Standards

### OpenFastTrack Format

Each requirement follows this structure:
```markdown
### REQ-XXX-N: Requirement Title
**Type**: [Functional/Non-functional/Interface]
**Priority**: [Mandatory/Important/Nice-to-have]  
**Description**: SHALL statement
**Rationale**: Why this requirement exists
**Acceptance Criteria**: Testable conditions (AC-1, AC-2...)
**Verification**: How to validate requirement
```

## Tools and Standards

- **OpenFastTrack (OFT)**: Requirements traceability and validation framework
- **GitHub Copilot**: AI-assisted implementation against formal specifications  
- **ESP-IDF v5.4.1**: ESP32 development framework and component system
- **Markdown**: Human-readable documentation with structured format
- **Git**: Version control with OFT-friendly plain text documentation

## Development Philosophy

This project demonstrates that **formal documentation and agile, incremental development are complementary**, showing how:

- **Structured requirements** enable better understanding and implementation
- **Component-based architecture** naturally maps to requirements decomposition  
- **Bidirectional traceability** ensures design decisions align with requirements
- **Pragmatic documentation** matches effort to complexity (web server = requirements-only)
- **Embedded systems** benefit from both engineering rigor and development efficiency

### Key Insights

1. **Requirements-driven development** works well for embedded systems with hardware constraints  
2. **Component documentation** matches ESP-IDF architecture patterns 
3. **Traceability matrices** provide valuable overview of system completeness
4. **Pragmatic documentation decisions** prevent over-engineering while maintaining structure

---

## Disclaimer

This documentation is **not intended to be exhaustive**. The ESP32 Distance Sensor project serves as a demonstration of the chosen requirements-driven approach and documentation methodology in principle. It is designed to illustrate process and structure, not to provide a complete or production-ready reference.

## Local docs workflow (devcontainer)

When working on documentation locally we recommend using the repository devcontainer and MkDocs for a reproducible preview experience.

- Start the devcontainer in VS Code (Reopen in Container). MkDocs and plugins should be installed in the container image or via the devcontainer `postCreateCommand`.
- To make the site homepage match the repository `README.md` (CI copies README -> `docs/index.md`), either edit `docs/index.md` directly, or copy the README before serving:

```bash
cp README.md docs/index.md
mkdocs serve --dev-addr 0.0.0.0:8000
```

- `mkdocs serve` watches `docs/` and `mkdocs.yml` and will automatically rebuild and trigger a browser reload when files change. Edit files under `docs/` for immediate live reload.
- If you prefer to keep `README.md` as the canonical homepage source, you can run a tiny watcher that copies `README.md` to `docs/index.md` on every save. A minimal script example is provided in the developer notes; running it in the container alongside `mkdocs serve` will keep the homepage in sync without restarting the server.

If you run into strict-mode build errors, fix the reported nav/link issues or run a non-strict build for quick previews:

```bash
mkdocs build    # build site to ./site
mkdocs serve    # serve without strict build
```

