#!/bin/bash
# QEMU ESP32 Emulator with Dual UART + GDB Debug Support
# 
# UART0 (tcp:5555)  → Main console (idf_monitor)
# UART1 (socket)    → LED visualization
# GDB   (tcp:3333)  → GDB remote debugging
#
# This script starts QEMU with GDB server enabled for debugging.

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

PROJECT_DIR="/workspaces/esp32-distance"
BUILD_DIR="${PROJECT_DIR}/build"
UART1_SOCKET="${PROJECT_DIR}/build/esp32-uart1.sock"
QEMU_TCP_PORT=5555
GDB_PORT=3333

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
    
    # Kill any nc processes connected to our socket
    pkill -f "nc.*${UART1_SOCKET}" 2>/dev/null || true
}

# Register cleanup on exit
trap cleanup EXIT

# Initial cleanup
cleanup

echo -e "${CYAN}╔════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║  ESP32 QEMU Emulator - Debug Mode with Dual UART      ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════════════════╝${NC}"
echo ""

# Check if firmware files exist
if [ ! -f "${BUILD_DIR}/qemu_flash.bin" ]; then
    echo -e "${RED}Error: QEMU flash image not found!${NC}"
    echo -e "${YELLOW}Run first: idf.py build${NC}"
    exit 1
fi

if [ ! -f "${BUILD_DIR}/qemu_efuse.bin" ]; then
    echo -e "${YELLOW}Creating efuse image...${NC}"
    dd if=/dev/zero of="${BUILD_DIR}/qemu_efuse.bin" bs=1024 count=4 2>/dev/null
fi

echo -e "${YELLOW}Starting QEMU in debug mode...${NC}"

# Start QEMU in background with GDB server enabled
qemu-system-xtensa \
    -M esp32 \
    -m 4M \
    -drive file=${BUILD_DIR}/qemu_flash.bin,if=mtd,format=raw \
    -drive file=${BUILD_DIR}/qemu_efuse.bin,if=none,format=raw,id=efuse \
    -global driver=nvram.esp32.efuse,property=drive,value=efuse \
    -global driver=timer.esp32.timg,property=wdt_disable,value=true \
    -nic user,model=open_eth \
    -nographic \
    -s -S \
    -serial tcp::${QEMU_TCP_PORT},server,nowait \
    -serial unix:${UART1_SOCKET},server,nowait &

# -s      = Start GDB server on port 1234 (default)
# -S      = Freeze CPU at startup (wait for GDB)
# But we use explicit -gdb tcp::3333 for custom port

QEMU_PID=$!

# Wait a moment for QEMU to start
sleep 2

# Check if QEMU is still running
if ! kill -0 ${QEMU_PID} 2>/dev/null; then
    echo -e "${RED}Error: QEMU failed to start!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ QEMU started successfully (PID: ${QEMU_PID})${NC}"
echo ""
echo -e "${CYAN}╔════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║  Connection Information                                ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${GREEN}UART Channels:${NC}"
echo -e "  ${BLUE}UART0:${NC} tcp://localhost:${QEMU_TCP_PORT} ${YELLOW}(console + monitor)${NC}"
echo -e "  ${BLUE}UART1:${NC} ${UART1_SOCKET} ${YELLOW}(LED visualization)${NC}"
echo ""
echo -e "${GREEN}GDB Debugging:${NC}"
echo -e "  ${BLUE}Port:${NC} localhost:1234 ${YELLOW}(QEMU -s default port)${NC}"
echo -e "  ${BLUE}Status:${NC} ${RED}CPU FROZEN${NC} - waiting for GDB to connect"
echo ""
echo -e "${CYAN}╔════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║  Quick Start Commands                                  ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${YELLOW}1. Start GDB in another terminal:${NC}"
echo -e "   ${GREEN}xtensa-esp32-elf-gdb build/distance.elf${NC}"
echo ""
echo -e "${YELLOW}2. In GDB, connect to QEMU:${NC}"
echo -e "   ${GREEN}(gdb) target remote :1234${NC}"
echo -e "   ${GREEN}(gdb) break app_main${NC}"
echo -e "   ${GREEN}(gdb) continue${NC}"
echo ""
echo -e "${YELLOW}3. View LED output in another terminal:${NC}"
echo -e "   ${GREEN}nc -U ${UART1_SOCKET}${NC}"
echo ""
echo -e "${YELLOW}4. View console logs (after GDB 'continue'):${NC}"
echo -e "   ${GREEN}nc localhost ${QEMU_TCP_PORT}${NC}"
echo ""
echo -e "${CYAN}╔════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║  Waiting for GDB connection...                         ║${NC}"
echo -e "${CYAN}║  Press Ctrl+C to abort                                 ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════════════════╝${NC}"
echo ""

# Wait for user to abort or QEMU to exit
wait ${QEMU_PID}
