# 📋 Planning Documents Overview

All planning documents ready for ESP32 template cleanup!

## 📚 Document Hierarchy

```text
temp/
├── GITHUB_CODING_AGENT_TASK.md (2.9K) ⭐ START HERE
├── CODING_AGENT_PLAN.md (19K) ⭐⭐ MAIN IMPLEMENTATION GUIDE
├── IMPLEMENTATION_SUMMARY.md (6.5K) - Executive summary & decisions
└── README.md (4.2K) - This overview document
```

## 🎯 For GitHub Coding Agent

**Start with:** `GITHUB_CODING_AGENT_TASK.md`  
**Main guide:** `CODING_AGENT_PLAN.md`

## 📖 Document Purposes

### 1. GITHUB_CODING_AGENT_TASK.md (START HERE)

- Quick task overview
- Reference to main plan
- Success criteria
- Critical requirements

**Use this to:** Trigger the coding agent

### 2. CODING_AGENT_PLAN.md (MAIN GUIDE)

- 7 detailed implementation phases
- Exact commands and file paths
- Code examples
- Testing procedures
- Success checklist

**Use this for:** Step-by-step implementation

### 3. IMPLEMENTATION_SUMMARY.md

- Final decisions summary
- Quick reference for components
- Testing checklist
- Known issues
- All questions answered

**Use this for:** Quick reference during implementation

## 🚀 Execution Flow

```text
1. Read: GITHUB_CODING_AGENT_TASK.md
   ↓
2. Follow: CODING_AGENT_PLAN.md (phases 1-7)
   ↓
3. Reference: IMPLEMENTATION_SUMMARY.md (when needed)
   ↓
4. Test: After each phase
   ↓
5. Verify: Success criteria checklist
```

## ✅ Key Decisions Implemented

| Area | Decision |
|------|----------|
| **Development** | Codespaces only |
| **Components** | Keep: config_manager, web_server, cert_handler, netif_uart_tunnel |
| **Remove** | distance_sensor, led_controller, display_logic, startup_tests, workshop/ |
| **Main.c** | Minimal Hello World with TODOs |
| **Documentation** | Keep requirements/design structure as template |
| **Legal** | Keep for GitHub Pages |
| **HTTPS** | Document as known issue (future work) |
| **Tools** | Keep all (especially http_proxy) |

## 📋 Implementation Phases

### Phase 1: Remove Components
- Delete 4 hardware components
- Remove workshop directory
- Clean project-specific docs
- Remove config.h

### Phase 2: Simplify Components
- Web server → Generic page + captive portal
- Config manager → Generic NVS examples
- Cert handler → Keep with TODO notes
- Network tunnel → Keep as-is

### Phase 3: Rewrite Core
- main.c → Minimal template
- CMakeLists.txt → Simplified dependencies
- Kconfig → Emulator flag only
- Root CMakeLists → Project name change

### Phase 4: Update Documentation
- README → Template introduction
- docs/index.md → Template overview
- Development docs → Codespaces focus
- Requirements/Design → Template guides

### Phase 5: GitHub Configuration
- Copilot instructions → Generic ESP32
- Prompt snippets → Remove distance sensor
- Keep workflows as-is

### Phase 6: MkDocs
- Update navigation
- Update site name
- Review page references

### Phase 7: Testing
- Build testing
- QEMU testing
- Documentation build
- Pre-commit hooks

## 🎯 Success Criteria

- [ ] Builds cleanly
- [ ] QEMU works with network
- [ ] HTTP proxy functional
- [ ] Docs build without errors
- [ ] Pre-commit passes
- [ ] No distance sensor references
- [ ] GitHub Pages ready
- [ ] Template is user-friendly

## 🔧 Testing Commands

```bash
# Build test
idf.py fullclean && idf.py build

# QEMU test
./tools/run-qemu-network.sh

# Documentation test
mkdocs build --strict

# Quality test
pre-commit run --all-files
```

## ⚠️ Known Issues

1. **HTTPS not working** - Document as limitation
2. **HTTP proxy required** - By design for QEMU access

## 📞 Next Steps

1. Review `GITHUB_CODING_AGENT_TASK.md`
2. Hand off to GitHub Coding Agent
3. Monitor implementation progress
4. Test after each phase
5. Verify final checklist

---

**All plans are ready!** 🎉

Hand off `GITHUB_CODING_AGENT_TASK.md` to start the implementation.

