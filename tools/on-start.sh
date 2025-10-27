#!/bin/bash
# on-start.sh

# VNC Startup Script for ESP32 Template
# This script ensures necessary permissions for VNC operation in the development container
sudo chmod 1777 /tmp

# Write timestamp to workspace root
cd /workspaces/esp32-template
echo "on-start.sh executed at $(date)" > .on-start-ran
