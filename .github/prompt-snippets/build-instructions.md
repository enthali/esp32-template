# Build Instructions for ESP32 Projects

This document provides detailed build, flash, and debugging instructions for ESP32 projects using the devcontainer environment.

## Prerequisites

- **ESP-IDF**: Version 5.4.1 (pre-installed in devcontainer)
- **Development Environment**: GitHub Codespaces or VS Code with Dev Containers
- **Hardware** (optional): ESP32 development board connected via USB
- **QEMU** (included): For testing without physical hardware

## Environment Setup

The devcontainer automatically sets up the ESP-IDF environment. All tools are ready to use immediately after container startup.

## Build Commands

### Standard Build Process

1. **Set Target** (first time only):

   ```bash
   idf.py set-target esp32
   ```

2. **Build Project**:

   ```bash
   idf.py build
   ```

3. **Flash to Hardware** (if connected):

   ```bash
   idf.py flash monitor
   ```

4. **Run in QEMU Emulator**:

   Use the VS Code task: `Run QEMU (No Debug)` or `Start QEMU Debug Server`

### Clean Build

For a completely clean build:

```bash
idf.py fullclean
idf.py build
```

## Troubleshooting

### Build Issues

1. **Module Not Found Errors**:
   - Verify all custom components are properly linked in `CMakeLists.txt`
   - Check component dependencies in `main/CMakeLists.txt`

2. **Component Errors**:
   - Ensure component structure follows ESP-IDF conventions
   - Check `CMakeLists.txt` in component directories

### Flash Issues (Hardware)

1. **Port Access Denied**:
   - In Codespaces: Hardware flashing not supported, use QEMU
   - In local devcontainer: Check USB passthrough configuration

2. **Flash Failed**:
   - Verify ESP32 is in download mode
   - Check USB cable connection
   - Try a different USB port or cable

### QEMU Issues

1. **QEMU Won't Start**:
   - Check if another QEMU instance is running: `pkill qemu`
   - Use VS Code task: `Stop QEMU` then restart

2. **Network Issues in QEMU**:
   - Verify `netif_uart_tunnel` component is enabled
   - Check QEMU network configuration in `tools/qemu/`

## Development Workflow

### Git Branch Management

When working on features or fixes:

```bash
# Create feature branch
git checkout -b feat/new-component

# Make changes and test
idf.py build

# Run in QEMU
# Use VS Code task: "Run QEMU (No Debug)"

# Commit changes
git add .
git commit -m "feat(component): Add new component"

# Push and create PR
git push origin feat/new-component
```

## Project Structure

### Component Organization

Custom components are located in `main/components/`:

- Add new components as subdirectories
- Each component needs its own `CMakeLists.txt`
- Follow ESP-IDF component structure conventions

### Main Application

Located in `main/` directory:

- `main.c`: Application entry point
- Component implementations in `main/components/`
- Configuration in `sdkconfig`

### Build Configuration

#### Flash Memory Configuration

This project is configured for ESP32 modules with **4MB flash memory**:

- **Flash Size**: 4MB (CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y)
- **Partition Table**: Single App Large (maximizes application space)
- **Free Flash**: ~41% available for application growth
- **HTTPS Ready**: Sufficient space for SSL/TLS certificates and HTTPS implementation

**Memory Usage Summary** (after optimization):

- Used flash: ~59% (vs 86% with 2MB config)
- Free flash: ~41% (vs 14% with 2MB config)
- IRAM usage: ~76% (sufficient headroom)
- DRAM usage: ~18% (low utilization, good for buffers)

#### Configuration Verification

To verify flash configuration:

```bash
idf.py size
```

To modify flash settings:

```bash
idf.py menuconfig
# Navigate to: Serial flasher config > Flash size
# Navigate to: Partition Table > Partition Table
```

#### Key Configuration Options

Key configuration options in `sdkconfig`:

- WiFi stack configuration
- FreeRTOS task stack sizes
- LED strip timing parameters
- Serial monitor baud rate
- **Flash size and partition table** (optimized for 4MB modules)

## Serial Monitor Commands

When monitoring device output (hardware or QEMU):

- **Ctrl+]**: Exit monitor
- **Ctrl+T, Ctrl+R**: Reset device
- **Ctrl+T, Ctrl+H**: Show help

## Debugging

### Using VS Code

1. **QEMU Debug**: Use launch configuration `Debug in QEMU`
2. **Hardware Debug**: Use launch configuration `Debug on ESP32` (requires hardware debugger)

### GDB Commands

The devcontainer includes ESP32-specific GDB tools:

```bash
# Start GDB session (QEMU must be running)
xtensa-esp32-elf-gdb build/esp32-template.elf
```

## Version Information

- **ESP-IDF**: v5.4.1 (pre-installed in devcontainer)
- **Target**: ESP32
- **Toolchain**: xtensa-esp32-elf (pre-installed)
- **QEMU**: ESP32 support included
- **Project Version**: Check `CMakeLists.txt` for current version

## Additional Resources

- **QEMU Tools**: See `tools/qemu/` for emulation scripts
- **Component Examples**: See `main/components/` for reference implementations
- **Documentation**: See `docs/` for Sphinx documentation
