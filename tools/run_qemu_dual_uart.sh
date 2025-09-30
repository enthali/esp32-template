#!/bin/bash
# QEMU ESP32 Emulator with Dual UART Setup
# 
# UART0 (tcp)    → Main console via idf_monitor (port 5555)
# UART1 (socket) → LED visualization channel
#
# This script mimics 'idf.py qemu monitor' but adds UART1 support.

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

PROJECT_DIR="/workspaces/esp32-distance"
BUILD_DIR="${PROJECT_DIR}/build"
UART1_SOCKET="${PROJECT_DIR}/build/esp32-uart1.sock"
QEMU_TCP_PORT=5555

# Cleanup function
cleanup() {
    echo -e "${YELLOW}Cleaning up...${NC}"
    
    # Kill QEMU if running
    pkill -f "qemu-system-xtensa.*esp32" 2>/dev/null || true
    
    # Remove UART1 socket if it exists
    if [ -S "${UART1_SOCKET}" ]; then
        rm -f "${UART1_SOCKET}"
        echo "Removed UART1 socket: ${UART1_SOCKET}"
    fi
    
    # Kill any socat processes connected to our socket
    pkill -f "socat.*${UART1_SOCKET}" 2>/dev/null || true
}

# Register cleanup on exit
trap cleanup EXIT

# Initial cleanup
cleanup

echo -e "${BLUE}=== ESP32 QEMU Emulator - Dual UART Mode ===${NC}"
echo -e "${GREEN}UART0:${NC} Main console (idf_monitor on tcp:${QEMU_TCP_PORT})"
echo -e "${GREEN}UART1:${NC} LED visualization → ${UART1_SOCKET}"
echo ""

# Check if firmware files exist
if [ ! -f "${BUILD_DIR}/qemu_flash.bin" ]; then
    echo -e "${RED}Error: QEMU flash image not found!${NC}"
    echo -e "${YELLOW}Run first: idf.py build${NC}"
    exit 1
fi

if [ ! -f "${BUILD_DIR}/qemu_efuse.bin" ]; then
    echo -e "${YELLOW}Creating efuse image...${NC}"
    # Create empty efuse image
    dd if=/dev/zero of="${BUILD_DIR}/qemu_efuse.bin" bs=1024 count=4 2>/dev/null
fi

echo -e "${YELLOW}Starting QEMU in background...${NC}"

# Start QEMU in background with dual UART configuration
qemu-system-xtensa \
    -M esp32 \
    -m 4M \
    -drive file=${BUILD_DIR}/qemu_flash.bin,if=mtd,format=raw \
    -drive file=${BUILD_DIR}/qemu_efuse.bin,if=none,format=raw,id=efuse \
    -global driver=nvram.esp32.efuse,property=drive,value=efuse \
    -global driver=timer.esp32.timg,property=wdt_disable,value=true \
    -nic user,model=open_eth \
    -nographic \
    -serial tcp::${QEMU_TCP_PORT},server,nowait \
    -serial unix:${UART1_SOCKET},server,nowait &

QEMU_PID=$!

# Wait a moment for QEMU to start and create socket
sleep 2

# Check if QEMU is still running
if ! kill -0 ${QEMU_PID} 2>/dev/null; then
    echo -e "${RED}Error: QEMU failed to start!${NC}"
    exit 1
fi

echo -e "${GREEN}QEMU started successfully (PID: ${QEMU_PID})${NC}"
echo ""
echo -e "${YELLOW}========================================${NC}"
echo -e "${GREEN}UART Channels:${NC}"
echo -e "  ${BLUE}UART0:${NC} tcp://localhost:${QEMU_TCP_PORT} (logs + monitor)"
echo -e "  ${BLUE}UART1:${NC} ${UART1_SOCKET} (LED visualization)"
echo ""
echo -e "${YELLOW}To view LED output in another terminal:${NC}"
echo -e "  ${BLUE}nc -U ${UART1_SOCKET}${NC}"
echo ""
echo -e "${YELLOW}For GDB debugging in another terminal:${NC}"
echo -e "  ${BLUE}xtensa-esp32-elf-gdb -ex 'target remote :3333' build/distance.elf${NC}"
echo -e "  ${GREEN}(QEMU GDB server runs on port 3333)${NC}"
echo ""
echo -e "${YELLOW}========================================${NC}"
echo ""
echo -e "${GREEN}Starting idf_monitor...${NC}"
echo -e "${BLUE}Press Ctrl+] to exit monitor${NC}"
echo ""

# Start idf_monitor in foreground (this will block until user exits)
idf.py -p socket://localhost:${QEMU_TCP_PORT} monitor

# When monitor exits, cleanup will be called automatically
echo ""
echo -e "${GREEN}Monitor exited. Cleaning up...${NC}"
