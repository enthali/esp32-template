#!/bin/bash
# QEMU ESP32 Emulator with Dual UART (Normal Mode - with idf_monitor)
# 
# Simple wrapper that uses idf.py qemu but adds UART1 for LED visualization

set -e

PROJECT_DIR="/workspaces/esp32-distance"
UART1_SOCKET="${PROJECT_DIR}/build/esp32-uart1.sock"

# Cleanup function
cleanup() {
    if [ -S "${UART1_SOCKET}" ]; then
        rm -f "${UART1_SOCKET}"
    fi
}

trap cleanup EXIT
cleanup

echo "=== ESP32 QEMU with Dual UART ==="
echo ""
echo "UART0: Console (idf_monitor)"
echo "UART1: LED visualization → ${UART1_SOCKET}"
echo ""
echo "To view LED output in another terminal:"
echo "  nc -U ${UART1_SOCKET}"
echo ""
echo "Starting QEMU with monitor..."
echo ""

# Use idf.py but with custom QEMU args via environment
export QEMU_EXTRA_ARGS="-serial unix:${UART1_SOCKET},server,nowait"

idf.py qemu monitor
