#!/bin/bash
# QEMU ESP32 Emulator with Graphics Display
# 
# This script runs QEMU with graphical output in addition to network support.
# The ESP32 display will be shown in a separate QEMU window in the GUI.

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Determine project directory (two levels up from tools/qemu/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
NETWORK_UART_PORT=5556

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}ESP32 QEMU - Graphics Mode${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Ensure we have X11 display set
if [ -z "$DISPLAY" ]; then
    echo -e "${YELLOW}Setting DISPLAY=:1 for GUI...${NC}"
    export DISPLAY=:1
fi

# Check if build is needed
BUILD_NEEDED=false
if [ ! -d "${PROJECT_DIR}/build" ]; then
    BUILD_NEEDED=true
    echo -e "${YELLOW}No build directory found${NC}"
elif [ ! -f "${PROJECT_DIR}/build/bootloader/bootloader.bin" ]; then
    BUILD_NEEDED=true
    echo -e "${YELLOW}Bootloader binary missing${NC}"
else
    # Extract project name for ELF check
    PROJECT_NAME=$(grep -oP 'set\(PROJECT_NAME\s+"?\K[^")]+' "${PROJECT_DIR}/CMakeLists.txt" | head -1 | tr -d ' ')
    if [ -z "$PROJECT_NAME" ]; then
        PROJECT_NAME="esp32-template"
    fi
    
    if [ ! -f "${PROJECT_DIR}/build/${PROJECT_NAME}.elf" ]; then
        BUILD_NEEDED=true
        echo -e "${YELLOW}ELF binary missing${NC}"
    fi
fi

# Build only if necessary
if [ "$BUILD_NEEDED" = true ]; then
    echo -e "${BLUE}🔨 Building project...${NC}"
    cd "${PROJECT_DIR}"
    idf.py build || {
        echo -e "${RED}Build failed!${NC}"
        exit 1
    }
    echo -e "${GREEN}✓ Build complete${NC}"
    echo ""
else
    echo -e "${GREEN}✓ Build artifacts found, skipping build${NC}"
    echo ""
fi

# Ensure network stack (TUN bridge + HTTP proxy) is running
echo -e "${YELLOW}Checking network infrastructure...${NC}"
"${PROJECT_DIR}/tools/network/ensure-network-stack.sh"
echo ""

echo -e "${YELLOW}Starting QEMU with Graphics Output...${NC}"
echo ""
echo -e "${GREEN}Network Configuration:${NC}"
echo -e "  UART0 Port: 5555 (Console/Monitor)"
echo -e "  UART1 Port: ${NETWORK_UART_PORT} (IP tunnel)"
echo -e "  ESP32 IP:   192.168.100.2"
echo -e "  Gateway:    192.168.100.1 (TUN device)"
echo ""
echo -e "${GREEN}Graphics Configuration:${NC}"
echo -e "  Display:    ${DISPLAY}"
echo -e "  Window:     QEMU ESP32 Monitor"
echo ""
echo -e "${YELLOW}Note:${NC} QEMU will wait for GDB to connect"
echo -e "${YELLOW}VS Code debugger will connect automatically${NC}"
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}QEMU is ready and waiting for GDB connection.${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Run QEMU via ESP-IDF with:
# - GDB debug support (-d flag makes it wait for debugger)
# - Two serial ports for network tunnel
# - SDL graphics output
# - Monitor console in graphics window

echo -e "${GREEN}Starting QEMU with SDL graphics display...${NC}"

# Start QEMU in background to capture output
idf.py qemu -d -g \
    --qemu-extra-args="-serial tcp::5555,server,nowait -serial tcp::${NETWORK_UART_PORT},server,nowait" &

QEMU_PID=$!

# Wait for QEMU to open GDB port (indicating it's ready)
echo -e "${YELLOW}Waiting for QEMU to start...${NC}"
for i in {1..30}; do
    if nc -z localhost 3333 2>/dev/null; then
        echo -e "${GREEN}✓ QEMU is ready and waiting for GDB connection${NC}"
        echo -e "${GREEN}✓ Graphics window should be visible in noVNC${NC}"
        echo "QEMU_READY_FOR_GDB"  # Plain text marker for VS Code task matcher - AFTER port check!
        break
    fi
    sleep 0.5
done

# Keep the script alive while QEMU runs
echo -e "${YELLOW}QEMU running with PID: ${QEMU_PID}${NC}"
echo -e "${YELLOW}Press Ctrl+C to stop QEMU${NC}"

# Wait for QEMU process to finish
wait $QEMU_PID