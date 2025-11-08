#!/bin/bash
# Simple helper to view UART0 output 
#
# Run this in a separate terminal while QEMU is running
# to see the LED strip visualization on a separate channel.

# Determine project directory (two levels up from tools/qemu/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
UART0_SOCKET="${PROJECT_DIR}/temp/esp32-uart0.sock"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}=== UART0 Monitor (Unix Socket) ===${NC}"
echo -e "Reading from: ${UART0_SOCKET} (QEMU UART0)"
echo -e "${YELLOW}Press Ctrl+C to stop${NC}"
echo ""

# Forever loop - survives QEMU restarts
while true; do
    # Wait for socket to exist
    WAIT_COUNT=0
    while [ ! -S "${UART0_SOCKET}" ]; do
        if [ $WAIT_COUNT -eq 0 ]; then
            echo "Waiting for QEMU to start and create socket..."
        fi
        sleep 1
        WAIT_COUNT=$((WAIT_COUNT + 1))
    done

    echo -e "${GREEN}Connected to UART0!${NC}"
    echo ""

    # Connect to socket using netcat (nc)
    # When QEMU stops/restarts, nc will exit and the loop will reconnect
    nc -U ${UART0_SOCKET}

    # Connection lost - QEMU probably stopped or restarted
    echo ""
    echo -e "${YELLOW}Connection lost. Reconnecting...${NC}"
    echo -e "${YELLOW}Press Ctrl+C to stop${NC}"
    echo ""
    sleep 1
done
