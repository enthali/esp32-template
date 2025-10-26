# 🚀 ESP32 Project Template

A production-ready ESP32 project template with GitHub Codespaces, QEMU emulation, and modern development tools.

**Documentation:** 👉 [View the Documentation on GitHub Pages](https://enthali.github.io/esp32-template/) 👈

This template provides a complete development environment for ESP32 projects with zero local setup required. Whether you're building an IoT device, learning embedded development, or creating a proof-of-concept—this template has everything you need to get started quickly.

---

## ✨ Features

- 🚀 **GitHub Codespaces** - Zero-setup cloud development environment
- 🖥️ **QEMU Emulation** - Test without hardware, includes network support
- 🐛 **GDB Debugging** - Full debugging in QEMU with VS Code integration
- 🌐 **Example Components** - Web server with captive portal, configuration management
- ⚙️ **Configuration Management** - NVS storage pattern examples
- 📝 **Documentation** - Sphinx with GitHub Pages deployment
- 🤖 **GitHub Copilot Ready** - AI-assisted development instructions included
- ✅ **Quality Gates** - Pre-commit hooks for linting and validation
- 📚 **Sphinx-Needs** - Requirements engineering with traceability matrices

## 🎯 Quick Start

### Use This Template

1. Click **"Use this template"** at the top of this repository
2. Create your new repository
3. Open in **GitHub Codespaces** (click Code → Codespaces → Create)
4. Wait for the environment to initialize (~2 minutes first time)
5. Build and run:

```bash
# Build the project
idf.py build

# Run in QEMU emulator
# Use VS Code task: "Start QEMU Debug Server"
# Or run manually:
./tools/run-qemu-network.sh
```

### Hardware Development

To flash a physical ESP32 device:

```bash
# Build the project
idf.py build

# Flash and monitor (replace /dev/ttyUSB0 with your port)
idf.py -p /dev/ttyUSB0 flash monitor

# Exit monitor: Press Ctrl+]
```

## 📁 Project Structure

```text
esp32-template/
├── main/                      # Main application code
│   ├── main.c                # Application entry point (customize this!)
│   ├── CMakeLists.txt        # Component dependencies
│   └── components/           # Optional example components
│       ├── config_manager/   # NVS configuration example
│       ├── web_server/       # HTTP server with captive portal
│       ├── cert_handler/     # HTTPS certificate handling (WIP)
│       └── netif_uart_tunnel/# QEMU network bridge
├── docs/                     # Sphinx documentation
│   ├── 11_requirements/     # Sphinx-Needs requirements
│   ├── 12_design/           # Design specifications  
│   ├── 21_api/              # API documentation
│   └── 31_traceability/     # Traceability matrices
├── tools/                    # Development tools
│   ├── run-qemu-network.sh  # QEMU with network bridge
│   └── http_proxy.py        # HTTP proxy for QEMU access
├── .devcontainer/           # GitHub Codespaces configuration
├── .github/                 # GitHub workflows and Copilot config
└── .vscode/                 # VS Code tasks and settings
```

## 🛠️ Customizing This Template

### 1. Update Project Metadata

- Edit `CMakeLists.txt`: Change `project(esp32-template)` to your project name
- Edit `README.md`: Update title, description, and features
- Edit `docs/index.md`: Update documentation homepage

### 2. Add Your Application Code

Start with `main/main.c` - this is your application entry point:

```c
void app_main(void)
{
    ESP_LOGI(TAG, "Your Application Starting...");
    
    // Initialize your components here
    // your_component_init();
    
    // Your application logic
    while (1) {
        // Your code here
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

### 3. Use Example Components (Optional)

Uncomment components in `main/CMakeLists.txt`:

```cmake
REQUIRES 
    nvs_flash
    config_manager    # Uncomment to use configuration management
    web_server        # Uncomment to use web server
```

### 4. Add Your Own Components

Create new components in `main/components/`:

```bash
mkdir -p main/components/my_component
# Add CMakeLists.txt, headers, and source files
```

## 🐛 QEMU Emulation & Debugging

### Run in QEMU

**With VS Code Tasks** (Recommended):

```bash
# Method 1: Debug Mode (waits for debugger)
# Use VS Code Task: "Start QEMU Debug Server"
# Then start debugger with F5

# Method 2: Run Mode (starts immediately)  
# Use VS Code Task: "Run QEMU (No Debug)"
```

**Manual Commands:**

```bash
# Debug mode (waits for GDB connection)
./tools/run-qemu-graphics.sh

# Run mode (starts immediately)
./tools/run-qemu-graphics-run.sh

# Legacy mode (network only, no graphics)
./tools/run-qemu-network.sh
```

**Graphics Output**: All QEMU modes include GUI output visible in NoVNC (port 6080). The ESP32 console, boot sequence, and any graphics will be displayed in the QEMU window within your browser.

**Network Access**: Use HTTP proxy for web interface:

```bash
# In another terminal, access web interface via HTTP proxy
python3 tools/http_proxy.py
# Then browse to http://localhost:8888
```

### Debug with GDB

1. Use VS Code task: **"Start QEMU Debug Server"**
2. Set breakpoints in your code
3. Use VS Code task: **"Debug ESP32 in QEMU"**
4. Step through code, inspect variables, etc.

See [Debugging Guide](docs/development/debugging.md) for details.

### GUI Development Environment

This template includes a full desktop environment for GUI applications and tools:

1. **Automatic Setup**: VNC is configured automatically when the container starts
2. **NoVNC Access**: Open GUI applications in your browser via port 6080
3. **Manual VNC Start**: Run `./tools/start-vnc.sh` if needed

```bash
# Start VNC server manually
./tools/start-vnc.sh

# Set display for GUI applications
export DISPLAY=:1

# Test GUI applications
xterm &
xclock &
xeyes &
```

**Access GUI**: In Codespaces, check the **PORTS** tab for port 6080 to open the desktop in your browser.

## 📚 Documentation

Full documentation is available at [GitHub Pages](https://enthali.github.io/esp32-template/) or build locally:

```bash
# Serve documentation locally
cd docs
sphinx-build -b html . _build/html
python -m http.server 8000 -d _build/html
# Browse to http://localhost:8000

# Build documentation
cd docs
sphinx-build -b html . _build/html
```

## 🤖 GitHub Copilot Integration

This template includes comprehensive GitHub Copilot instructions in `.github/copilot-instructions.md`:

- ESP32-specific coding standards
- Component architecture patterns
- Memory optimization guidelines
- Sphinx-Needs requirements methodology
- Build and testing workflows

Just ask Copilot for help and it will use these project-specific guidelines!

## ✅ Quality Gates

Pre-commit hooks ensure code quality:

```bash
# Run all checks
pre-commit run --all-files

# Install hooks (automatic in Codespaces)
pre-commit install
```

Checks include:

- Markdown linting
- Sphinx documentation build validation
- Link verification
- Trailing whitespace removal

## 🌐 Example Components

### Configuration Manager

NVS-based configuration storage with defaults:

```c
#include "config_manager.h"

// Initialize and load configuration
config_init();
system_config_t config;
config_get_current(&config);

// Update configuration
config.wifi_ssid = "MyNetwork";
config_save(&config);
```

### Web Server

HTTP server with captive portal:

- Landing page at `/`
- Configuration API at `/api/config`
- Captive portal for WiFi setup
- Mobile-responsive interface

## 📖 Requirements Engineering

This template uses **Sphinx-Needs** for professional requirements management:

- **Requirements** in `docs/11_requirements/` - System and component requirements
- **Design** in `docs/12_design/` - Design specifications
- **API Reference** in `docs/21_api/` - Code documentation
- **Traceability** in `docs/31_traceability/` - Auto-generated relationship graphs

Features:

- ✅ Unique requirement IDs with automatic validation
- ✅ Bidirectional traceability links
- ✅ Visual dependency graphs (needflow)
- ✅ Filterable requirement tables
- ✅ Coverage analysis and statistics

See [Requirements Documentation](https://enthali.github.io/esp32-template/11_requirements/) for details.

## 🚧 Known Limitations

- **HTTPS**: Certificate handling is work-in-progress (HTTP works fine)
- **QEMU Network**: Requires HTTP proxy for web access from host
- **Codespaces Only**: Local dev container setup not officially supported

## 📄 License

This project template is open source and available under the MIT License.

## 🎉 About

ESP32 embedded development template featuring:

- **ESP-IDF v5.4.1** - Espressif IoT Development Framework
- **FreeRTOS** - Real-time operating system
- **QEMU** - Full system emulation with networking
- **Sphinx-Needs** - Requirements engineering with traceability
- **GitHub Codespaces** - Cloud-based development
- **GitHub Copilot** - AI-assisted development

---

**Ready to build something awesome?**

Fork this template, customize it for your project, and start coding! If you have questions or suggestions, open an issue or start a discussion.

**Happy coding!** 🚀
