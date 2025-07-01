# ESP32 Build Commands

## Windows Environment Setup

**Important**: Always use capital `C:` for the drive letter when working with MCP filesystem tools.

### Complete Build Command
```powershell
cmd /c "cd /D C:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py build"
```

### Flash and Monitor
```powershell
cmd /c "cd /D C:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py flash monitor"
```

### Individual Commands

**Build Only**:
```powershell
cmd /c "cd /D C:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py build"
```

**Flash Only**:
```powershell
cmd /c "cd /D C:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py flash"
```

**Monitor Only**:
```powershell
cmd /c "cd /D C:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py monitor"
```

**Memory Analysis**:
```powershell
cmd /c "cd /D C:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py size"
```

**Configuration Menu**:
```powershell
cmd /c "cd /D C:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py menuconfig"
```

**Clean Build**:
```powershell
cmd /c "cd /D C:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py fullclean && idf.py build"
```

## Command Breakdown

**Command Structure**:
- `cmd /c`: Execute command in Windows Command Prompt
- `cd /D C:\workspace\ESP32_Projects\distance`: Change to project directory (capital C!)
- `C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat`: Setup ESP-IDF environment
- `idf.py [command]`: Execute ESP-IDF command

## Target Configuration

**Set Target** (first time setup):
```powershell
cmd /c "cd /D C:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py set-target esp32"
```

## Memory Configuration

Current project is configured for:
- **Flash Size**: 4MB (ESP32 WROOM-32F)
- **Partition Table**: Single App Large
- **Available Flash**: ~41% free for future features

## Serial Monitor Controls

When using `idf.py monitor`:
- **Exit**: `Ctrl+]`
- **Reset Device**: `Ctrl+T, Ctrl+R`
- **Help**: `Ctrl+T, Ctrl+H`

## Troubleshooting

### Environment Issues
- Verify ESP-IDF installation path is correct
- Ensure ESP-IDF version 5.4.1 is installed
- Check that Python dependencies are installed

### Build Issues
- Run `idf.py fullclean` before rebuilding
- Check component dependencies in CMakeLists.txt
- Verify all custom components are properly linked

### Flash Issues
- Close any open serial monitor applications
- Check USB connection and driver installation
- Verify correct COM port assignment

## Project-Specific Notes

### Custom Components
- `components/distance_sensor/`: HC-SR04 sensor interface
- `components/led_controller/`: WS2812 LED strip control

### Configuration Files
- `sdkconfig`: Main project configuration (version controlled)
- `CMakeLists.txt`: Build system configuration
- `main/CMakeLists.txt`: Main application dependencies