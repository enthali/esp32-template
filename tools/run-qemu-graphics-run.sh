#!/bin/bash
# QEMU ESP32 Emulator with Graphics Display (No Debug Mode)
# 
# This script runs QEMU with graphical output WITHOUT waiting for GDB.
# The ESP32 will start immediately and you can see the console output.

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Determine project directory (script location parent directory)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
NETWORK_UART_PORT=5556

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}ESP32 QEMU - Graphics Mode (No Debug)${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Ensure we have X11 display set
if [ -z "$DISPLAY" ]; then
    echo -e "${YELLOW}Setting DISPLAY=:1 for GUI...${NC}"
    export DISPLAY=:1
fi

# Build project first (delta build is fast, ensures latest code)
echo -e "${YELLOW}Building project...${NC}"
cd "${PROJECT_DIR}"
idf.py build || {
    echo -e "${RED}Build failed!${NC}"
    exit 1
}
echo ""

# Ensure network stack (TUN bridge + HTTP proxy) is running
echo -e "${YELLOW}Checking network infrastructure...${NC}"
"${PROJECT_DIR}/tools/ensure-network-stack.sh"
echo ""

echo -e "${YELLOW}Starting QEMU with Graphics Output (No Debug)...${NC}"
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
echo -e "${YELLOW}Note:${NC} ESP32 will start immediately (no GDB wait)"
echo -e "${YELLOW}You should see boot messages and web server startup${NC}"
echo ""
echo -e "${BLUE}========================================${NC}"
echo ""

# Run QEMU via ESP-IDF with:
# - NO debug mode (ESP32 starts immediately)
# - Graphics enabled
# - Two serial ports for network tunnel

echo -e "${GREEN}Starting QEMU with graphics display (running mode)...${NC}"

# Start QEMU WITHOUT -d flag (no debug wait)
idf.py qemu -g \
    --qemu-extra-args="-serial tcp::5555,server,nowait -serial tcp::${NETWORK_UART_PORT},server,nowait" &

QEMU_PID=$!

# Wait a moment for QEMU to start
echo -e "${YELLOW}Waiting for QEMU to start...${NC}"
sleep 2

# Check if QEMU is still running
if ps -p $QEMU_PID > /dev/null; then
    echo -e "${GREEN}✓ QEMU started successfully with PID: ${QEMU_PID}${NC}"
    echo -e "${GREEN}✓ Graphics window should show ESP32 boot sequence${NC}"
    echo -e "${GREEN}✓ ESP32 will start WiFi and web server automatically${NC}"
else
    echo -e "${RED}✗ QEMU failed to start${NC}"
    exit 1
fi

# Keep the script alive while QEMU runs
echo -e "${YELLOW}QEMU running with PID: ${QEMU_PID}${NC}"
echo -e "${YELLOW}Press Ctrl+C to stop QEMU${NC}"
echo ""
echo -e "${GREEN}Expected ESP32 behavior:${NC}"
echo -e "  1. Boot sequence visible in graphics window"
echo -e "  2. WiFi AP mode: 'ESP32-Template'"
echo -e "  3. Web server: http://192.168.100.2:80"
echo -e "  4. Captive portal for configuration"

# Wait for QEMU process to finish
wait $QEMU_PID