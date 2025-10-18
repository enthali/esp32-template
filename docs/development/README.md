# Development Environment

This section covers all aspects of developing with the ESP32 Template, from setting up your environment to debugging and testing.

## Quick Links

- **[Dev Container Setup](devcontainer.md)** - GitHub Codespaces (recommended)
- **[QEMU Emulator](qemu-emulator.md)** - Running in QEMU with network support
- **[Debugging](debugging.md)** - GDB debugging with QEMU and VS Code
- **[Network Internals](qemu-network-internals.md)** - Deep dive into QEMU IP tunnel implementation
- **[Pre-commit Hooks](pre-commit-hooks.md)** - Automated quality checks and documentation validation

## Getting Started

### Recommended: GitHub Codespaces

The fastest way to start developing:

1. Click "Use this template" to create your repository
2. Open in GitHub Codespaces (Code → Codespaces → Create)
3. Wait for container to build (first time only, ~2 minutes)
4. Start coding in `main/main.c`

### Testing Without Hardware

Use QEMU emulation:

```bash
# Build the project
idf.py build

# Run in QEMU with network support
./tools/run-qemu-network.sh

# Debug in QEMU
# Press F5 in VS Code or use Debug panel
```

### Flashing Real Hardware

When you have physical ESP32:

```bash
# Build the project
idf.py build

# Flash and monitor (replace /dev/ttyUSB0 with your port)
idf.py -p /dev/ttyUSB0 flash monitor

# Exit monitor: Press Ctrl+]
```

## Development Workflow

```bash
# 1. Customize application
vim main/main.c

# 2. Build
idf.py build

# 3. Test in QEMU (optional)
./tools/run-qemu-network.sh

# 4. Flash to hardware
idf.py -p /dev/ttyUSB0 flash monitor
```

## Template Customization

- **Consistent Environment**: Same toolchain everywhere (Codespaces, Docker, CI/CD)
- **ESP-IDF v5.4.1**: Latest stable version pre-configured
- **QEMU Support**: Test without hardware using full network emulation
- **GDB Debugging**: Full breakpoint debugging in emulator
- **Web Interface**: Access emulated web server via browser
- **Quality Gates**: Pre-commit hooks validate documentation and code before commits

## Next Steps

- New to dev containers? Start with [Dev Container Setup](devcontainer.md)
- Want to debug? Check out [Debugging Guide](debugging.md)
- Curious about QEMU networking? See [QEMU Emulator](qemu-emulator.md)
- Need technical details? Read [Network Internals](qemu-network-internals.md)
- Setting up quality checks? See [Pre-commit Hooks](pre-commit-hooks.md)

## Known Issues

Unfortunately ESP-IDF-WEB doesn't automatically install. please search the extension in the marketplace and install it.
