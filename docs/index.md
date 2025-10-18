# Welcome to the ESP32 Project Template

A production-ready ESP32 project template with GitHub Codespaces, QEMU emulation, and modern development tools.

## 🚀 Why Use This Template?

Starting an ESP32 project from scratch can be overwhelming. This template provides:

- ✅ **Zero-setup development** - Open in GitHub Codespaces and start coding immediately
- ✅ **No hardware required** - Test with QEMU emulation before buying hardware
- ✅ **Professional structure** - Component-based architecture following ESP-IDF best practices
- ✅ **Quality gates** - Pre-commit hooks ensure code quality from day one
- ✅ **Documentation ready** - MkDocs with GitHub Pages deployment included
- ✅ **AI-assisted development** - GitHub Copilot instructions for ESP32-specific guidance

## 🎯 Perfect For

- 🔨 **Prototyping** - Quickly test ideas with QEMU before committing to hardware
- 📚 **Learning** - Clean example code demonstrating ESP32 development patterns
- 🏢 **Production projects** - Professional structure ready to scale
- 👥 **Team collaboration** - Consistent environment for all developers
- 🎓 **Teaching** - Pre-configured environment for workshops and courses

## 🚀 Quick Start

### 1. Create Your Repository

Click **"Use this template"** at the top of the [GitHub repository](https://github.com/enthali/esp32-template) to create your own project.

### 2. Open in Codespaces

- Click **Code** → **Codespaces** → **Create codespace on main**
- Wait ~2 minutes for environment setup (first time only)
- Everything is ready to use!

### 3. Build Your First Project

```bash
# Build the minimal template
idf.py build

# Run in QEMU emulator
./tools/run-qemu-network.sh

# Or flash to real hardware
idf.py -p /dev/ttyUSB0 flash monitor
```

## 📁 What's Included

### Example Components

- **Configuration Manager** - NVS storage patterns for persistent settings
- **Web Server** - HTTP server with captive portal for WiFi setup
- **Certificate Handler** - HTTPS support (work in progress)
- **Network Bridge** - UART tunnel for QEMU networking

### Development Tools

- **QEMU Emulator** - Full ESP32 emulation with network support
- **GDB Debugging** - Set breakpoints and step through code
- **HTTP Proxy** - Access emulated web servers from host
- **Pre-commit Hooks** - Automated code quality checks

### Documentation

- **Requirements** - OpenFastTrack requirements engineering structure
- **Design** - System architecture and component design
- **Development Guides** - QEMU, debugging, and environment setup
- **API Documentation** - Structure ready for API doc generation

## 🛠️ Customization Guide

### Update Project Metadata

1. **CMakeLists.txt** - Change project name from `esp32-template`
2. **README.md** - Update title and description
3. **docs/index.md** - Update documentation homepage
4. **mkdocs.yml** - Update site name and description

### Add Your Application Logic

Edit `main/main.c` - this is your entry point:

```c
void app_main(void)
{
    ESP_LOGI(TAG, "Your Application Starting...");
    
    // Initialize NVS (already included)
    // Add your component initialization here
    // Start your application logic
    
    while (1) {
        // Your main loop
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

### Use Example Components

Uncomment components in `main/CMakeLists.txt`:

```cmake
REQUIRES 
    nvs_flash
    config_manager    # Uncomment to use
    web_server        # Uncomment to use
```

See component directories for usage examples.

### Create New Components

```bash
mkdir -p main/components/my_component
# Add CMakeLists.txt, headers, and source files
# Follow existing components as examples
```

## 🧪 Development Workflow

### QEMU Emulation

Test without hardware:

```bash
# Build for emulator
idf.py build

# Start QEMU with network
./tools/run-qemu-network.sh

# In another terminal, start HTTP proxy
python3 tools/http_proxy.py

# Access web interface at http://localhost:8888
```

### Hardware Testing

Flash to real ESP32:

```bash
# Build project
idf.py build

# Flash and monitor
idf.py -p /dev/ttyUSB0 flash monitor

# Exit monitor: Ctrl+]
```

### Debugging

Use VS Code tasks:

1. **Start QEMU Debug Server** - Launches QEMU with GDB
2. **Debug ESP32 in QEMU** - Attaches debugger
3. Set breakpoints and step through code

See [Debugging Guide](development/debugging.md) for details.

## 📚 Documentation

### For Developers

- **[QEMU Emulator](development/qemu-emulator.md)** - Emulation setup and usage
- **[Debugging](development/debugging.md)** - GDB debugging guide
- **[Dev Container](development/devcontainer.md)** - Codespaces configuration
- **[Pre-commit Hooks](development/pre-commit-hooks.md)** - Quality gates

### For Architects

- **[Requirements](requirements/README.md)** - OpenFastTrack methodology
- **[Design](design/README.md)** - System design documentation
- **[Architecture](architecture/README.md)** - Component architecture

### Example Documentation

- **[Config Requirements](requirements/config-requirements.md)** - Example requirements doc
- **[Config Design](design/config-design.md)** - Example design doc
- **[Web Server Requirements](requirements/web-server-requirements.md)** - Another example

## 🤖 GitHub Copilot Integration

This template includes comprehensive instructions for GitHub Copilot in `.github/copilot-instructions.md`:

- ESP32-specific coding standards (memory management, error handling)
- Component architecture patterns
- QEMU and Codespaces workflows
- OpenFastTrack requirements methodology
- Build and testing procedures

Just ask Copilot for help and it will follow these guidelines!

## ✅ Quality Gates

Pre-commit hooks ensure quality:

```bash
# Run all checks
pre-commit run --all-files

# Checks include:
# - Markdown linting
# - MkDocs build validation  
# - Link verification
# - File encoding
```

CI/CD automatically runs these checks on pull requests.

## 🎓 Learning Resources

### ESP-IDF Documentation

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [API Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/index.html)
- [FreeRTOS](https://www.freertos.org/Documentation/RTOS_book.html)

### This Template

- Browse example components in `main/components/`
- Read inline code comments for usage patterns
- Check requirements and design docs for methodology

## 🌟 Template Features

| Feature | Status | Description |
|---------|--------|-------------|
| **GitHub Codespaces** | ✅ Ready | Zero-setup cloud development |
| **QEMU Emulation** | ✅ Ready | Test without hardware |
| **GDB Debugging** | ✅ Ready | Full debugging support |
| **Example Components** | ✅ Ready | Config manager, web server |
| **Pre-commit Hooks** | ✅ Ready | Automated quality checks |
| **GitHub Pages** | ✅ Ready | Documentation deployment |
| **Copilot Instructions** | ✅ Ready | AI-assisted development |
| **OpenFastTrack Docs** | ✅ Ready | Requirements engineering |
| **HTTPS Support** | 🚧 WIP | Certificate handling in progress |

## 🎯 Next Steps

1. **Explore the code** - Check out `main/main.c` and example components
2. **Build and run** - Try the QEMU emulator
3. **Read the docs** - Browse the development guides
4. **Customize** - Make it your own!
5. **Deploy** - Push to GitHub for automatic documentation deployment

## 💬 Community & Support

Questions? Suggestions? Found a bug?

- Open an [issue](https://github.com/enthali/esp32-template/issues)
- Start a [discussion](https://github.com/enthali/esp32-template/discussions)
- Check existing documentation

This is a community project - contributions are welcome!

**Happy coding!** 🚀
