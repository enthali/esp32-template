#!/bin/bash
# QEMU ESP32 Emulator with Dual UART + GDB Debug Support
# 
# UART0 (tcp)    → Main console via idf_monitor (port 5555)
# UART1 (socket) → LED visualization channel
# GDB Server     → Port 3333 (use with xtensa-esp32-elf-gdb)
#
# This script is based on ESP-IDF's qemu_ext.py implementation.

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
QEMU_GDB_PORT=3333

# Cleanup function
cleanup() {
    echo -e "\n${YELLOW}Cleaning up...${NC}"
    
    # Kill QEMU if running (using PID if available)
    if [ ! -z "${QEMU_PID}" ] && kill -0 ${QEMU_PID} 2>/dev/null; then
        kill ${QEMU_PID} 2>/dev/null || true
        wait ${QEMU_PID} 2>/dev/null || true
        echo "Stopped QEMU (PID: ${QEMU_PID})"
    else
        pkill -f "qemu-system-xtensa.*esp32" 2>/dev/null && echo "Stopped QEMU" || true
    fi
    
    # Remove UART1 socket if it exists
    if [ -S "${UART1_SOCKET}" ]; then
        rm -f "${UART1_SOCKET}"
        echo "Removed UART1 socket"
    fi
}

# Register cleanup on exit/interrupt
trap cleanup EXIT INT TERM

# Initial cleanup
cleanup 2>/dev/null || true

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}ESP32 QEMU - Dual UART + GDB Debug${NC}"
echo -e "${BLUE}========================================${NC}"

# Check if firmware files exist
if [ ! -f "${BUILD_DIR}/qemu_flash.bin" ]; then
    echo -e "${RED}Error: QEMU flash image not found!${NC}"
    echo -e "${YELLOW}Generating flash image...${NC}"
    cd "${PROJECT_DIR}"
    idf.py build
fi

# Generate proper efuse for ESP32 rev 3 (following ESP-IDF qemu_ext.py)
if [ ! -f "${BUILD_DIR}/qemu_efuse.bin" ]; then
    echo -e "${YELLOW}Creating efuse image (ESP32 rev 3)...${NC}"
    # ESP32 chip revision 3 efuse values (from ESP-IDF qemu_ext.py)
    python3 << 'EOF'
import binascii
efuse_hex = (
    '00000000000000000000000000800000000000000000100000000000000000000000000000000000'
    '00000000000000000000000000000000000000000000000000000000000000000000000000000000'
    '00000000000000000000000000000000000000000000000000000000000000000000000000000000'
    '00000000'
)
with open('build/qemu_efuse.bin', 'wb') as f:
    f.write(binascii.unhexlify(efuse_hex))
print("Created: build/qemu_efuse.bin")
EOF
fi

echo ""
echo -e "${YELLOW}Starting QEMU in background with GDB server...${NC}"

# Start QEMU in background with:
# - Dual UART (tcp + unix socket)
# - GDB server on port 3333
# - Wait for GDB to connect (-S flag)
qemu-system-xtensa \
    -M esp32 \
    -m 4M \
    -drive file=${BUILD_DIR}/qemu_flash.bin,if=mtd,format=raw \
    -drive file=${BUILD_DIR}/qemu_efuse.bin,if=none,format=raw,id=efuse \
    -global driver=nvram.esp32.efuse,property=drive,value=efuse \
    -global driver=timer.esp32.timg,property=wdt_disable,value=true \
    -nic user,model=open_eth \
    -nographic \
    -gdb tcp::${QEMU_GDB_PORT} \
    -S \
    -serial tcp::${QEMU_TCP_PORT},server,nowait \
    -serial tcp::5556,server,nowait &

QEMU_PID=$!

# Wait for QEMU to be ready (check if TCP port is open)
echo -n "Waiting for QEMU to start"
for i in {1..30}; do
    if nc -z localhost ${QEMU_TCP_PORT} 2>/dev/null; then
        echo -e " ${GREEN}✓${NC}"
        break
    fi
    echo -n "."
    sleep 0.3
    if [ $i -eq 30 ]; then
        echo -e "\n${RED}Timeout waiting for QEMU!${NC}"
        exit 1
    fi
done

# Check if QEMU is still running
if ! kill -0 ${QEMU_PID} 2>/dev/null; then
    echo -e "${RED}Error: QEMU failed to start!${NC}"
    exit 1
fi

echo -e "${GREEN}QEMU started successfully (PID: ${QEMU_PID})${NC}"

# Wait for GDB port to be ready
echo -n "Waiting for GDB server to be ready"
for i in {1..30}; do
    if nc -z localhost ${QEMU_GDB_PORT} 2>/dev/null; then
        echo -e " ${GREEN}✓${NC}"
        break
    fi
    echo -n "."
    sleep 0.2
    if [ $i -eq 30 ]; then
        echo -e "\n${RED}Timeout waiting for GDB port!${NC}"
        exit 1
    fi
done

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${YELLOW}UART Channels:${NC}"
echo -e "  ${GREEN}UART0:${NC} tcp://localhost:${QEMU_TCP_PORT} (logs + monitor)"
echo -e "  ${GREEN}UART1:${NC} ${UART1_SOCKET} (LED visualization)"
echo ""
echo -e "${YELLOW}GDB Debug:${NC}"
echo -e "  ${GREEN}Port:${NC} ${QEMU_GDB_PORT}"
echo -e "  ${GREEN}Command:${NC} xtensa-esp32-elf-gdb -ex 'target remote :${QEMU_GDB_PORT}' build/distance.elf"
echo ""
echo -e "${RED}⚠ QEMU is waiting for GDB to connect!${NC}"
echo -e "${YELLOW}To start execution:${NC}"
echo -e "  1. Open another terminal"
echo -e "  2. Run: ${BLUE}xtensa-esp32-elf-gdb build/distance.elf${NC}"
echo -e "  3. In GDB: ${BLUE}target remote :${QEMU_GDB_PORT}${NC}"
echo -e "  4. In GDB: ${BLUE}continue${NC} (or ${BLUE}c${NC})"
echo ""
echo -e "${YELLOW}To view LED output in another terminal:${NC}"
echo -e "  nc -U ${UART1_SOCKET}"
echo ""
echo -e "${BLUE}========================================${NC}"
echo ""

# Wait for user to press Ctrl+C
echo -e "${YELLOW}Press Ctrl+C to stop QEMU...${NC}"
wait ${QEMU_PID}
